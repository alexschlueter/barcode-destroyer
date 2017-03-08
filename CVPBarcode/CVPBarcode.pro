#-------------------------------------------------
#
# Project created by QtCreator 2016-11-21T17:55:20
#
#-------------------------------------------------

TEMPLATE = subdirs
SUBDIRS = gui \
          cli

DISTFILES += \
    common.pri

CONFIG += ordered

RESOURCES = cells.qrc

SUBDIRS += \
    cli/cli.pro \
    gui/gui.pro
