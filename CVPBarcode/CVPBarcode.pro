#-------------------------------------------------
#
# Project created by QtCreator 2016-11-21T17:55:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CVPBarcode
TEMPLATE = app

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

    INCLUDEPATH += /usr/include

    LIBS += -L/usr/local/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}

HEADERS += \
    Pipeline/gradientblurpipeline.h \
    Pipeline/pipeline.h \
    Steps/loaderstep.h \
    Steps/readerstep.h \
    Steps/showstep.h \
    Steps/step.h \
    mainwindow.h \
    Steps/gradientblurstep.h

SOURCES += \
    Pipeline/gradientblurpipeline.cpp \
    Pipeline/pipeline.cpp \
    Steps/loaderstep.cpp \
    Steps/readerstep.cpp \
    Steps/showstep.cpp \
    Steps/step.cpp \
    main.cpp \
    mainwindow.cpp \
    Steps/gradientblurstep.cpp
