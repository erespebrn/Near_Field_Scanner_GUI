QT       += \
    core gui \
    multimediawidgets

QT += multimedia multimediawidgets

win32:RC_ICONS += sduicon.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += C:\opencv\build\include

INCLUDEPATH += C:\Program Files\Rohde-Schwarz\RsVisa

LIBS += C:\opencv\release\bin\libopencv_core420.dll
LIBS += C:\opencv\release\bin\libopencv_highgui420.dll
LIBS += C:\opencv\release\bin\libopencv_imgcodecs420.dll
LIBS += C:\opencv\release\bin\libopencv_imgproc420.dll
LIBS += C:\opencv\release\bin\libopencv_features2d420.dll
LIBS += C:\opencv\release\bin\libopencv_calib3d420.dll
LIBS += C:\opencv\release\bin\libopencv_videoio420.dll
LIBS += C:\opencv\release\bin\libopencv_objdetect420.dll
LIBS += C:\opencv\release\bin\libopencv_photo420.dll

LIBS += C:/Windows/System32/visa32.dll

SOURCES += \
    instrument_thread.cpp \
    main.cpp \
    qlabel_mouseevent.cpp \
    scan_settings.cpp \
    scanner_gui.cpp \
    videothread.cpp

HEADERS += \
    instrument_thread.h \
    qlabel_mouseevent.h \
    scan_settings.h \
    scanner_gui.h \
    videothread.h

FORMS += \
    scan_settings.ui \
    scanner_gui.ui

RESOURCES += camera.qrc \
    img.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/camera
INSTALLS += target

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

