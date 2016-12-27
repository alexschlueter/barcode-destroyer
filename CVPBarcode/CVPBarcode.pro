#-------------------------------------------------
#
# Project created by QtCreator 2016-11-21T17:55:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CVPBarcode
TEMPLATE = app
CONFIG += c++14

win32 {
    # Windows Compiler Flags

    QMAKE_CXXFLAGS += -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas

    INCLUDEPATH += C:/openCV31/include

    LIBS += -LC:/openCV31/bin \
            -lopencv_core310 \
            -lopencv_highgui310 \
            -lopencv_imgproc310 \
            -lopencv_imgcodecs310

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

unix {

    QMAKE_CXXFLAGS += -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas -Wno-unused-variable -Wno-reorder -Wno-maybe-uninitialized

    LIBS += -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs
}

HEADERS += \
    Pipeline/gradientblurpipeline.h \
    Pipeline/pipeline.h \
    Steps/loaderstep.h \
    Steps/readerstep.h \
    Steps/showstep.h \
    Steps/step.h \
    mainwindow.h \
    Steps/gradientblurstep.h \
    Pipeline/lsdpipeline.h \
    Steps/lsdstep.h \
    Steps/templatematchingstep.h \
    aspectratiopixmaplabel.h \
    Steps/utils.h

SOURCES += \
    Pipeline/gradientblurpipeline.cpp \
    Pipeline/pipeline.cpp \
    Steps/loaderstep.cpp \
    Steps/readerstep.cpp \
    Steps/showstep.cpp \
    Steps/step.cpp \
    main.cpp \
    mainwindow.cpp \
    Steps/gradientblurstep.cpp \
    Pipeline/lsdpipeline.cpp \
    Steps/lsdstep.cpp \
    Steps/templatematchingstep.cpp \
    aspectratiopixmaplabel.cpp \
    Steps/utils.cpp
