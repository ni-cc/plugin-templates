include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginminimal)

message(============================================)
message(Qt version: $$[QT_VERSION])
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginminimal.cpp \

HEADERS += \
    devicepluginminimal.h \
