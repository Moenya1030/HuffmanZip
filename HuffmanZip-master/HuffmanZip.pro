#-------------------------------------------------
#
# Project created by QtCreator 2020-12-11T16:36:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HuffmanZip
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    huffmanTree.cpp \
    zip.cpp

HEADERS += \
    mainwindow.h \
    huffmanTree.h \
    zip.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    images.qrc

DISTFILES += \
    background.jpg \
    background.jpg

#RC_FILE = myapp.rc
