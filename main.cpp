#include "RecognitionBridge.h"
#include "qvideostream.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<RecognitionResult>("RecognitionResult");

    QVideoStream::registerTypes();

    RecognitionBridge recognition;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("recognition"), &recognition);
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    return app.exec();
}
