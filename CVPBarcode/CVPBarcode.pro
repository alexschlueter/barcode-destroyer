#-------------------------------------------------
#
# Project created by QtCreator 2016-11-21T17:55:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CVPBarcode
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    detector.cpp

HEADERS  += mainwindow.h \
    detector.h

macx {

    # MAC Compiler Flags
}

win32 {
    # Windows Compiler Flags

    QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas

    INCLUDEPATH += C:/openCV/include

    LIBS += -LC:/openCV/bin \
            -lopencv_core2413 \
            -lopencv_highgui2413 \
            -lopencv_imgproc2413

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

unix {

    QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas

    INCLUDEPATH += /home/alex/opencv/include

    LIBS += -L/home/alex/opencv/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}
