HEADERS       = window.h \
    config.h
SOURCES       = main.cpp \
    config.cpp \
    window.cpp

RESOURCES     = watch.qrc


QT += network widgets macextras
ICON = logo.icns

TARGET = watch
TEMPLATE = app





