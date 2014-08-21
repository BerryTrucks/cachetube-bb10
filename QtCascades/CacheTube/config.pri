# Config.pri file version 2.0. Auto-generated by IDE. Any changes made by user will be lost!
BASEDIR = $$quote($$_PRO_FILE_PWD_)

device {
    CONFIG(debug, debug|release) {
        profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        } else {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }

    }

    CONFIG(release, debug|release) {
        !profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }
    }
}

simulator {
    CONFIG(debug, debug|release) {
        !profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }
    }
}

config_pri_assets {
    OTHER_FILES += \
        $$quote($$BASEDIR/assets/PlayerPage.qml) \
        $$quote($$BASEDIR/assets/doc/help.html) \
        $$quote($$BASEDIR/assets/doc/help_ru.html) \
        $$quote($$BASEDIR/assets/doc/review_app_button.png) \
        $$quote($$BASEDIR/assets/images/back.png) \
        $$quote($$BASEDIR/assets/images/cache.png) \
        $$quote($$BASEDIR/assets/images/cache_empty.png) \
        $$quote($$BASEDIR/assets/images/cachetube.png) \
        $$quote($$BASEDIR/assets/images/delete.png) \
        $$quote($$BASEDIR/assets/images/home.png) \
        $$quote($$BASEDIR/assets/images/pause.png) \
        $$quote($$BASEDIR/assets/images/play.png) \
        $$quote($$BASEDIR/assets/images/reload.png) \
        $$quote($$BASEDIR/assets/images/review.png) \
        $$quote($$BASEDIR/assets/images/youtube.png) \
        $$quote($$BASEDIR/assets/main.qml)
}

config_pri_source_group1 {
    SOURCES += \
        $$quote($$BASEDIR/src/CacheTube.cpp) \
        $$quote($$BASEDIR/src/appsettings.cpp) \
        $$quote($$BASEDIR/src/customtimer.cpp) \
        $$quote($$BASEDIR/src/main.cpp) \
        $$quote($$BASEDIR/src/ytarraydatamodel.cpp) \
        $$quote($$BASEDIR/src/ytvideomanager.cpp)

    HEADERS += \
        $$quote($$BASEDIR/src/CacheTube.hpp) \
        $$quote($$BASEDIR/src/appsettings.h) \
        $$quote($$BASEDIR/src/customtimer.h) \
        $$quote($$BASEDIR/src/ytarraydatamodel.h) \
        $$quote($$BASEDIR/src/ytvideomanager.h)
}

INCLUDEPATH += $$quote($$BASEDIR/src)

CONFIG += precompile_header

PRECOMPILED_HEADER = $$quote($$BASEDIR/precompiled.h)

lupdate_inclusion {
    SOURCES += \
        $$quote($$BASEDIR/../src/*.c) \
        $$quote($$BASEDIR/../src/*.c++) \
        $$quote($$BASEDIR/../src/*.cc) \
        $$quote($$BASEDIR/../src/*.cpp) \
        $$quote($$BASEDIR/../src/*.cxx) \
        $$quote($$BASEDIR/../assets/*.qml) \
        $$quote($$BASEDIR/../assets/*.js) \
        $$quote($$BASEDIR/../assets/*.qs) \
        $$quote($$BASEDIR/../assets/doc/*.qml) \
        $$quote($$BASEDIR/../assets/doc/*.js) \
        $$quote($$BASEDIR/../assets/doc/*.qs) \
        $$quote($$BASEDIR/../assets/images/*.qml) \
        $$quote($$BASEDIR/../assets/images/*.js) \
        $$quote($$BASEDIR/../assets/images/*.qs)

    HEADERS += \
        $$quote($$BASEDIR/../src/*.h) \
        $$quote($$BASEDIR/../src/*.h++) \
        $$quote($$BASEDIR/../src/*.hh) \
        $$quote($$BASEDIR/../src/*.hpp) \
        $$quote($$BASEDIR/../src/*.hxx)
}

TRANSLATIONS = $$quote($${TARGET}_ru.ts) \
    $$quote($${TARGET}.ts)
