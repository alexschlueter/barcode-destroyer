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
            -lopencv_imgcodecs310 \
            -lopencv_ml310

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

unix {

    QMAKE_CXXFLAGS += -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-reorder -Wno-maybe-uninitialized

    LIBS += -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_ml
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
    Steps/lsdstep.h \
    Steps/templatematchingstep.h \
    aspectratiopixmaplabel.h \
    utils.h \
    Steps/neuralhoughstep.h \
    Steps/neural-hough/HoughTransform.hpp \
    Steps/neural-hough/mlp_threshold.hpp \
    Steps/neural-hough/MLP.hpp \
    Steps/neural-hough/hough_histogram.hpp \
    Pipeline/neuralhoughpipeline.h \
    Steps/neural-hough/draw_hist.hpp \
    Steps/variationboundaryfinderstep.h \
    Steps/muensterboundaryfinderstep.h \
    Pipeline/lsdtemplatepipeline.h \
    Pipeline/lsdmuenstertemplatepipeline.h \
    Pipeline/lsdlinebndtemplatepipeline.h \
    Steps/lsdboundaryfinderstep.h

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
    Steps/lsdstep.cpp \
    Steps/templatematchingstep.cpp \
    aspectratiopixmaplabel.cpp \
    utils.cpp \
    Steps/neuralhoughstep.cpp \
    Steps/neural-hough/HoughTransform.cpp \
    Steps/neural-hough/mlp_threshold.cpp \
    Steps/neural-hough/MLP.cpp \
    Steps/neural-hough/hough_histogram.cpp \
    Pipeline/neuralhoughpipeline.cpp \
    Steps/neural-hough/draw_hist.cpp \
    Steps/variationboundaryfinderstep.cpp \
    Steps/muensterboundaryfinderstep.cpp \
    Pipeline/lsdtemplatepipeline.cpp \
    Pipeline/lsdmuenstertemplatepipeline.cpp \
    Pipeline/lsdlinebndtemplatepipeline.cpp \
    Steps/lsdboundaryfinderstep.cpp
