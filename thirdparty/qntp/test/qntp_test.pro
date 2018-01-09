#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T17:22:17
#
#-------------------------------------------------

include(../qntp.pri)

QT       += network testlib

QT       -= gui

TARGET = qntp_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += qntp_test.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
