QT += core gui qml quick quickcontrols2

TEMPLATE = app
TARGET = FaceRecognitionLibUsage

CONFIG += c++17 release
CONFIG += no_mocdepend
CONFIG -= debug app_bundle
CONFIG -= depend_includepath

CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/debug
} else {
    DESTDIR = $$OUT_PWD/release
}

INCLUDEPATH += \
    $$PWD/external_includes/facerecognition/src/api \
    $$PWD/external_includes/facerecognition/src/inference \
    $$PWD/external_includes/facerecognition/src/vision \
    $$PWD/external_includes/qvideostream \
    $$PWD/external_includes/opencv \
    $$PWD/external_includes/onnxruntime \
    $$PWD/external_includes/ffmpeg

LIBS += \
    -L$$PWD/external_libs/facerecognition/lib -lFaceRecognition \
    -L$$PWD/external_libs/qvideostream/lib -lQVideoStream \
    -L$$PWD/external_libs/opencv/lib -lopencv_world4130

SOURCES += \
    main.cpp \
    RecognitionBridge.cpp

HEADERS += \
    RecognitionBridge.h

RESOURCES += qml.qrc

win32 {
    PWD_WIN = $$replace(PWD, /, \\)
    DESTDIR_WIN = $$replace(DESTDIR, /, \\)

    QMAKE_POST_LINK += $$quote(cmd /c if not exist \"$$DESTDIR_WIN\" mkdir \"$$DESTDIR_WIN\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c if not exist \"$$DESTDIR_WIN\\models\" mkdir \"$$DESTDIR_WIN\\models\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c if not exist \"$$DESTDIR_WIN\\face_embeddings\" mkdir \"$$DESTDIR_WIN\\face_embeddings\" $$escape_expand(\\n\\t))

    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\facerecognition\\bin\\*.dll\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\qvideostream\\bin\\*.dll\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\ffmpeg\\bin\\*.dll\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\ffmpeg\\bin\\*.exe\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\onnxruntime\\bin\\*.dll\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\external_libs\\opencv\\bin\\*.dll\" \"$$DESTDIR_WIN\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\models\\*.*\" \"$$DESTDIR_WIN\\models\\\" $$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$PWD_WIN\\face_embeddings\\*.*\" \"$$DESTDIR_WIN\\face_embeddings\\\" $$escape_expand(\\n\\t))
}
