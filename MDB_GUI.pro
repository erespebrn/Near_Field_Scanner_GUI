QT       += \
    core gui \
    network \
    serialport \
    printsupport \

win32:RC_ICONS += sduicon.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

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

LIBS += C:\opencv\release\bin\libopencv_core420.dll
LIBS += C:\opencv\release\bin\libopencv_highgui420.dll
LIBS += C:\opencv\release\bin\libopencv_imgcodecs420.dll
LIBS += C:\opencv\release\bin\libopencv_imgproc420.dll
LIBS += C:\opencv\release\bin\libopencv_features2d420.dll
LIBS += C:\opencv\release\bin\libopencv_calib3d420.dll
LIBS += C:\opencv\release\bin\libopencv_videoio420.dll
LIBS += C:\opencv\release\bin\libopencv_objdetect420.dll
LIBS += C:\opencv\release\bin\libopencv_photo420.dll


SOURCES += \
    Scan_process/Event_log/event_log.cpp \
    Scan_process/Instrument_settings/scan_settings.cpp \
    Scan_process/Mouse_events/livestream_mouse_ev.cpp \
    Scan_process/PCB_size/dut_size.cpp \
    Scan_process/Plotting/nf_plot.cpp \
    Scan_process/Plotting/nf_plot_sa.cpp \
    Scan_process/Plotting/nf_plot_static.cpp \
    Scan_process/Plotting/nf_plot_vna.cpp \
    Scan_process/Plotting/qcustomplot.cpp \
    Scan_process/Scan_wizard/scan_interface.cpp \
    Scan_process/Scan_wizard/scanarea.cpp \
    Scan_process/Scan_wizard/scanwizard.cpp \
    Scan_process/Tool_add/tool_add.cpp \
    Serial/powermgm.cpp \
    TCP_IP/RS_Instr/rs_instr_detector_thread.cpp \
    TCP_IP/RS_Instr/rs_instruments.cpp \
    TCP_IP/RS_Instr/rs_sa.cpp \
    TCP_IP/RS_Instr/rs_vna.cpp \
    TCP_IP/Robot/robot.cpp \
    TCP_IP/tcp_device.cpp \
    main.cpp \
    mainsettings.cpp \
    scanner_gui.cpp \
    videothread.cpp

HEADERS += \
    Scan_process/Event_log/event_log.h \
    Scan_process/Instrument_settings/scan_settings.h \
    Scan_process/Mouse_events/livestream_mouse_ev.h \
    Scan_process/PCB_size/dut_size.h \
    Scan_process/Plotting/nf_plot.h \
    Scan_process/Plotting/nf_plot_sa.h \
    Scan_process/Plotting/nf_plot_static.h \
    Scan_process/Plotting/nf_plot_vna.h \
    Scan_process/Plotting/qcustomplot.h \
    Scan_process/Scan_wizard/scan_interface.h \
    Scan_process/Scan_wizard/scanarea.h \
    Scan_process/Scan_wizard/scanwizard.h \
    Scan_process/Tool_add/tool_add.h \
    Serial/powermgm.h \
    TCP_IP/RS_Instr/rs_instr_detector_thread.h \
    TCP_IP/RS_Instr/rs_instruments.h \
    TCP_IP/RS_Instr/rs_sa.h \
    TCP_IP/RS_Instr/rs_vna.h \
    TCP_IP/Robot/robot.h \
    TCP_IP/tcp_device.h \
    mainsettings.h \
    scanner_gui.h \
    videothread.h

FORMS += \
    Forms/mainsettings.ui \
    Forms/nf_plot.ui \
    Forms/scan_settings.ui \
    Forms/scanner_gui.ui \
    Forms/scanwizard.ui \
    Forms/tool_add.ui \
    Forms/scanarea.ui \
    Forms/powermgm.ui \

RESOURCES += \
    img.qrc \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    images/green.png \
    images/home_icon.svg \
    images/led_off.png \
    images/led_on.png \
    images/probe_rotate.png \
    images/rotate.png \
    images/shutter.svg

