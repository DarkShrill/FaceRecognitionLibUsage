#include "RecognitionBridge.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QVariantMap>

#include <opencv2/imgproc.hpp>

namespace {
QVariantMap pointToMap(const cv::Point2f& point) {
    QVariantMap map;
    map.insert(QStringLiteral("x"), point.x);
    map.insert(QStringLiteral("y"), point.y);
    return map;
}

QVariantMap pointToMap(const cv::Point3f& point) {
    QVariantMap map;
    map.insert(QStringLiteral("x"), point.x);
    map.insert(QStringLiteral("y"), point.y);
    map.insert(QStringLiteral("z"), point.z);
    return map;
}

QVariantList fivePointsToList(const std::array<cv::Point2f, 5>& points) {
    QVariantList list;
    list.reserve(static_cast<int>(points.size()));
    for (const cv::Point2f& point : points) {
        list.append(pointToMap(point));
    }
    return list;
}

QVariantList pointsToList(const std::vector<cv::Point2f>& points) {
    QVariantList list;
    list.reserve(static_cast<int>(points.size()));
    for (const cv::Point2f& point : points) {
        list.append(pointToMap(point));
    }
    return list;
}

QVariantList pointsToList(const std::vector<cv::Point3f>& points) {
    QVariantList list;
    list.reserve(static_cast<int>(points.size()));
    for (const cv::Point3f& point : points) {
        list.append(pointToMap(point));
    }
    return list;
}
}

RecognitionBridge::RecognitionBridge(QObject* parent)
    : QObject(parent) {}

RecognitionBridge::~RecognitionBridge() = default;

QString RecognitionBridge::sourceUrl() const {
    return m_sourceUrl;
}

void RecognitionBridge::setSourceUrl(const QString& sourceUrl) {
    const QString cleaned = sourceUrl.trimmed();
    if (m_sourceUrl == cleaned) {
        return;
    }

    m_sourceUrl = cleaned;
    emit sourceUrlChanged();
}

QString RecognitionBridge::statusText() const {
    return m_statusText;
}

QString RecognitionBridge::summaryText() const {
    return m_summaryText;
}

QVariantList RecognitionBridge::faces() const {
    return m_faces;
}

int RecognitionBridge::frameWidth() const {
    return m_frameWidth;
}

int RecognitionBridge::frameHeight() const {
    return m_frameHeight;
}

bool RecognitionBridge::isInitialized() const {
    return m_initialized;
}

bool RecognitionBridge::isBusy() const {
    return m_busy;
}

bool RecognitionBridge::useCuda() const {
    return m_useCuda;
}

void RecognitionBridge::setUseCuda(bool useCuda) {
    if (m_useCuda == useCuda) {
        return;
    }

    m_useCuda = useCuda;
    emit useCudaChanged();
}

int RecognitionBridge::landmarkMode() const {
    return m_landmarkMode;
}

void RecognitionBridge::setLandmarkMode(int landmarkMode) {
    if (m_landmarkMode == landmarkMode) {
        return;
    }

    m_landmarkMode = landmarkMode;
    if (m_engine) {
        m_engine->setLandmarkMode(m_landmarkMode);
    }
    emit landmarkModeChanged();
}

bool RecognitionBridge::initialize() {
    QStringList missingFiles;
    if (!runtimeFilesAvailable(&missingFiles)) {
        m_engine.reset();
        m_initialized = false;
        setBusy(false);
        emit initializedChanged();
        setStatusText(QStringLiteral("File runtime mancanti: %1").arg(missingFiles.join(QStringLiteral(", "))));
        return false;
    }

    m_engine = std::make_unique<FaceRecognitionEngine>(
        m_useCuda ? InferenceDevice::CUDA : InferenceDevice::CPU);
    connectEngine();
    m_engine->setLandmarkMode(m_landmarkMode);

    const bool ok = m_engine->initialize(
        runtimePath(QStringLiteral("models/det_500m.onnx")),
        runtimePath(QStringLiteral("models/2d106det.onnx")),
        runtimePath(QStringLiteral("models/1k3d68.onnx")),
        runtimePath(QStringLiteral("models/w600k_mbf.onnx")),
        runtimePath(QStringLiteral("face_embeddings")));

    if (m_initialized != ok) {
        m_initialized = ok;
        emit initializedChanged();
    }

    setBusy(false);
    setStatusText(ok
                      ? QStringLiteral("Pronto (%1)").arg(m_useCuda ? QStringLiteral("CUDA richiesta") : QStringLiteral("CPU"))
                      : QStringLiteral("Inizializzazione fallita"));
    return ok;
}

void RecognitionBridge::submitFrame(const QImage& image) {
    if (!m_initialized || !m_engine || m_busy || image.isNull()) {
        return;
    }

    if (m_frameWidth != image.width() || m_frameHeight != image.height()) {
        m_frameWidth = image.width();
        m_frameHeight = image.height();
        emit frameSizeChanged();
    }

    const QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat rgbMat(rgb.height(),
                   rgb.width(),
                   CV_8UC3,
                   const_cast<uchar*>(rgb.constBits()),
                   static_cast<size_t>(rgb.bytesPerLine()));

    cv::Mat bgr;
    cv::cvtColor(rgbMat, bgr, cv::COLOR_RGB2BGR);
    m_engine->submitFrame(bgr);
}

void RecognitionBridge::connectEngine() {
    connect(m_engine.get(), &FaceRecognitionEngine::busyChanged, this, [this](bool busy) {
        setBusy(busy);
    });

    connect(m_engine.get(), &FaceRecognitionEngine::resultReady, this, &RecognitionBridge::handleResult);
}

void RecognitionBridge::setStatusText(const QString& statusText) {
    if (m_statusText == statusText) {
        return;
    }

    m_statusText = statusText;
    emit statusTextChanged();
}

void RecognitionBridge::setSummaryText(const QString& summaryText) {
    if (m_summaryText == summaryText) {
        return;
    }

    m_summaryText = summaryText;
    emit summaryTextChanged();
}

void RecognitionBridge::setBusy(bool busy) {
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

QString RecognitionBridge::runtimePath(const QString& relativePath) const {
    const QDir appDir(QCoreApplication::applicationDirPath());
    return QDir::toNativeSeparators(appDir.filePath(relativePath));
}

bool RecognitionBridge::runtimeFilesAvailable(QStringList* missingFiles) const {
    const QStringList requiredFiles = {
        QStringLiteral("models/det_500m.onnx"),
        QStringLiteral("models/face_landmark_2d_106.onnx"),
        QStringLiteral("models/1k3d68.onnx"),
        QStringLiteral("models/w600k_mbf.onnx"),
        QStringLiteral("face_embeddings")
    };

    QStringList missing;
    for (const QString& relativePath : requiredFiles) {
        if (!QFileInfo::exists(runtimePath(relativePath))) {
            missing.append(relativePath);
        }
    }

    if (missingFiles) {
        *missingFiles = missing;
    }
    return missing.isEmpty();
}

void RecognitionBridge::handleResult(const RecognitionResult& result) {
    QVariantList faces;
    faces.reserve(static_cast<int>(result.faces.size()));

    for (const DetectedFace& face : result.faces) {
        QVariantMap map;
        map.insert(QStringLiteral("x"), face.bbox.x);
        map.insert(QStringLiteral("y"), face.bbox.y);
        map.insert(QStringLiteral("w"), face.bbox.width);
        map.insert(QStringLiteral("h"), face.bbox.height);
        map.insert(QStringLiteral("name"), face.name);
        map.insert(QStringLiteral("confidence"), face.confidencePercent);
        map.insert(QStringLiteral("landmarks5"), fivePointsToList(face.kps));
        map.insert(QStringLiteral("landmarks106"), pointsToList(face.landmarks106));
        map.insert(QStringLiteral("landmarks3d68"), pointsToList(face.landmarks3d68));
        faces.append(map);
    }

    m_faces = faces;
    emit facesChanged();

    setSummaryText(QStringLiteral("%1 volti | FPS %2 | luce %3")
                       .arg(result.faces.size())
                       .arg(result.fps, 0, 'f', 1)
                       .arg(result.brightness, 0, 'f', 0));
}
