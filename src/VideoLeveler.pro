#-------------------------------------------------
#
# Project created by QtCreator 2013-12-16T17:06:52
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = VideoLeveler
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS_DEBUG += -g
QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -DNDEBUG
QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/lib -lopencv_core -lopencv_highgui -lglog -lboost_filesystem -lboost_system -lboost_program_options -lopencv_imgproc

SOURCES += main.cpp
