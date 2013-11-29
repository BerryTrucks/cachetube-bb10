APP_NAME = CacheTube

CONFIG += qt warn_on cascades10

LIBS += -lbb -lbbsystem

TRANSLATIONS = $${TARGET}_ru.ts \
               $${TARGET}.ts

# Uncomment this to enable debugging output to console
DEFINES += DEBUG_CONSOLE

include(config.pri)
