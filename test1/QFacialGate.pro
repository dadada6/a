QT += widgets

CONFIG += c++11 console

TARGET = QFacialGate
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
# disables all the APIs deprecated before Qt 6.0.0

#LIBS += -lrkfacial -lrga -lpthread -lIPCProtocol

#INCLUDEPATH +=

QMAKE_CXXFLAGS += -fpermissive -g

SOURCES += main.cpp \
    videoitem.cpp \
    desktopview.cpp \
    savethread.cpp \
    snapshotthread.cpp \
    qtkeyboard.cpp \
    startitem.cpp \
    waterprocess.cpp

HEADERS += \
    videoitem.h \
    desktopview.h \
    savethread.h \
    snapshotthread.h \
    qtkeyboard.h \
    startitem.h \
    waterprocess.h

FORMS += \
    qtkeyboard.ui \

RESOURCES += \
    QFacialGate.qrc

TRANSLATIONS += zh_CN.ts \
                en_US.ts

