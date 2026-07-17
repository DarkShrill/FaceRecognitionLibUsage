#pragma once

#include "FaceRecognitionEngine.h"

#include <QImage>
#include <QObject>
#include <QString>
#include <QVariantList>

#include <memory>

class RecognitionBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sourceUrl READ sourceUrl WRITE setSourceUrl NOTIFY sourceUrlChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString summaryText READ summaryText NOTIFY summaryTextChanged)
    Q_PROPERTY(QVariantList faces READ faces NOTIFY facesChanged)
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY frameSizeChanged)
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY frameSizeChanged)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(bool useCuda READ useCuda WRITE setUseCuda NOTIFY useCudaChanged)
    Q_PROPERTY(int landmarkMode READ landmarkMode WRITE setLandmarkMode NOTIFY landmarkModeChanged)

public:
    explicit RecognitionBridge(QObject* parent = nullptr);
    ~RecognitionBridge() override;

    QString sourceUrl() const;
    void setSourceUrl(const QString& sourceUrl);

    QString statusText() const;
    QString summaryText() const;
    QVariantList faces() const;
    int frameWidth() const;
    int frameHeight() const;
    bool isInitialized() const;
    bool isBusy() const;
    bool useCuda() const;
    void setUseCuda(bool useCuda);
    int landmarkMode() const;
    void setLandmarkMode(int landmarkMode);

    Q_INVOKABLE bool initialize();
    Q_INVOKABLE void submitFrame(const QImage& image);

signals:
    void sourceUrlChanged();
    void statusTextChanged();
    void summaryTextChanged();
    void facesChanged();
    void frameSizeChanged();
    void initializedChanged();
    void busyChanged();
    void useCudaChanged();
    void landmarkModeChanged();

private:
    void connectEngine();
    void setStatusText(const QString& statusText);
    void setSummaryText(const QString& summaryText);
    void setBusy(bool busy);
    QString runtimePath(const QString& relativePath) const;
    bool runtimeFilesAvailable(QStringList* missingFiles) const;
    void handleResult(const RecognitionResult& result);

    std::unique_ptr<FaceRecognitionEngine> m_engine;
    QString m_sourceUrl = QStringLiteral("video=Full HD webcam");
    QString m_statusText;
    QString m_summaryText = QStringLiteral("Nessun frame");
    QVariantList m_faces;
    int m_frameWidth = 0;
    int m_frameHeight = 0;
    bool m_initialized = false;
    bool m_busy = false;
    bool m_useCuda = false;
    int m_landmarkMode = static_cast<int>(LandmarkMode::All);
};
