QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += core gui network widgets sql

DEFINES += QT_DEPRECATED_WARNINGS
QT += network
QT += widgets
QT += core gui
QT += core-private

# Для отладки
CONFIG += debug console

CONFIG += c++17

SOURCES += \
    client.cpp \
    main.cpp \
    mainwindow.cpp \
    reg_and_auth.cpp \

HEADERS += \
    client.h \
    mainwindow.h \
    reg_and_auth.h \

FORMS += \
    mainwindow.ui \
    reg_and_auth.ui \



qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
