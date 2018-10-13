#-------------------------------------------------
#
# Project created by QtCreator 2017-01-25T17:39:19
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ancamgui
TEMPLATE = app

TRANSLATIONS = ancam_ja.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        widget.cpp \
    cameradataaccess.cpp \
    previewwidget.cpp \
    configwidget.cpp \
    cameradatamodel.cpp \
    filesortfilterproxymodel.cpp \
    selectdatedialog.cpp \
    imageviewerwidget.cpp \
    serversettingdialog.cpp \
    servermanager.cpp \
    servermanagermodel.cpp \
    filedeletedialog.cpp

HEADERS  += widget.h \
    cameradataaccess.h \
    previewwidget.h \
    configwidget.h \
    cameradatamodel.h \
    filesortfilterproxymodel.h \
    selectdatedialog.h \
    imageviewerwidget.h \
    serversettingdialog.h \
    servermanager.h \
    servermanagermodel.h \
    filedeletedialog.h

FORMS    += widget.ui \
    previewwidget.ui \
    configwidget.ui \
    selectdatedialog.ui \
    serversettingdialog.ui \
    filedeletedialog.ui

RESOURCES += \
    resource.qrc

DISTFILES += \
    win.rc

win32 {
    RC_FILE = win.rc
}

# install
target.path = /bin
INSTALLS += target
