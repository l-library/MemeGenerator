QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    commands.cpp \
    cyberdistressingdialog.cpp \
    filterdialog.cpp \
    imagecropperlabel.cpp \
    main.cpp \
    mainwindow.cpp \
    menuconfig.cpp \
    resizableitem.cpp

HEADERS += \
    commands.h \
    cyberdistressingdialog.h \
    dimoutsidecanvaseffect.h \
    filterdialog.h \
    imagecropperdialog.h \
    imagecropperlabel.h \
    mainwindow.h \
    menuconfig.h \
    resizableitem.h

FORMS += \
    cyberdistressingdialog.ui \
    filterdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc


RC_FILE += logo.rc
