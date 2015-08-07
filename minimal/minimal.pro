include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginminimal)

message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginminimal.cpp \

HEADERS += \
    devicepluginminimal.h \
