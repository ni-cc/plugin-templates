include(plugins.pri)

TARGET = guh_devicepluginnetworkinfo

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginnetworkinfo.cpp \

HEADERS += \
    devicepluginnetworkinfo.h \
