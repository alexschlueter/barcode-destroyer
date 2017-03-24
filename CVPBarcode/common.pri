QT  += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++14
QMAKE_CXXFLAGS += -Wall -Werror -Wextra -pedantic -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-reorder -Wno-maybe-uninitialized -Wno-format-security

TEMPLATE = app

RESOURCES = ../cells.qrc

win32 {

    INCLUDEPATH += C:/openCV31/include

    LIBS += -LC:/openCV31/bin \
            -lopencv_core310 \
            -lopencv_highgui310 \
            -lopencv_imgproc310 \
            -lopencv_imgcodecs310 \
            -lopencv_ml310
}

unix {

    LIBS += -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_ml
}

HEADERS += \
    ../Pipeline/gradientblurpipeline.h \
    ../Pipeline/pipeline.h \
    ../Steps/loaderstep.h \
    ../Steps/step.h \
    ../Steps/gradientblurstep.h \
    ../Steps/lsdstep.h \
    ../Steps/templatematchingstep.h \
    ../aspectratiopixmaplabel.h \
    ../utils.h \
    ../Steps/variationboundaryfinderstep.h \
    ../Steps/muensterboundaryfinderstep.h \
    ../Pipeline/lsdtemplatepipeline.h \
    ../Pipeline/lsdmuenstertemplatepipeline.h \
    ../Pipeline/lsdlinebndtemplatepipeline.h \
    ../Steps/lsdboundaryfinderstep.h

SOURCES += \
    ../Pipeline/gradientblurpipeline.cpp \
    ../Pipeline/pipeline.cpp \
    ../Steps/loaderstep.cpp \
    ../Steps/step.cpp \
    ../Steps/gradientblurstep.cpp \
    ../Steps/lsdstep.cpp \
    ../Steps/templatematchingstep.cpp \
    ../aspectratiopixmaplabel.cpp \
    ../utils.cpp \
    ../Steps/variationboundaryfinderstep.cpp \
    ../Steps/muensterboundaryfinderstep.cpp \
    ../Pipeline/lsdtemplatepipeline.cpp \
    ../Pipeline/lsdmuenstertemplatepipeline.cpp \
    ../Pipeline/lsdlinebndtemplatepipeline.cpp \
    ../Steps/lsdboundaryfinderstep.cpp
