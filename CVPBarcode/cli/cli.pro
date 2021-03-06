# Check if the config file exists
! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

SOURCES += \
    main_cli.cpp \
    solutionfilewriter.cpp


# By default, TARGET is the same as the directory, so it will make
# liblogic.a (in linux).  Uncomment to override.
# TARGET = target

TARGET = CVPBarcodeCLI

HEADERS += \
    solutionfilewriter.h
