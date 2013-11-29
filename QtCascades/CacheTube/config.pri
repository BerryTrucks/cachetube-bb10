# Auto-generated by IDE. Any changes made by user will be lost!
BASEDIR =  $$quote($$_PRO_FILE_PWD_)

device {
    CONFIG(debug, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/CacheTube.cpp) \
                 $$quote($$BASEDIR/src/appsettings.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/playerlauncher.cpp) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.cpp) \
                 $$quote($$BASEDIR/src/ytvideomanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/CacheTube.hpp) \
                 $$quote($$BASEDIR/src/appsettings.h) \
                 $$quote($$BASEDIR/src/playerlauncher.h) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.h) \
                 $$quote($$BASEDIR/src/ytvideomanager.h)
    }

    CONFIG(release, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/CacheTube.cpp) \
                 $$quote($$BASEDIR/src/appsettings.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/playerlauncher.cpp) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.cpp) \
                 $$quote($$BASEDIR/src/ytvideomanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/CacheTube.hpp) \
                 $$quote($$BASEDIR/src/appsettings.h) \
                 $$quote($$BASEDIR/src/playerlauncher.h) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.h) \
                 $$quote($$BASEDIR/src/ytvideomanager.h)
    }
}

simulator {
    CONFIG(debug, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/CacheTube.cpp) \
                 $$quote($$BASEDIR/src/appsettings.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/playerlauncher.cpp) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.cpp) \
                 $$quote($$BASEDIR/src/ytvideomanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/CacheTube.hpp) \
                 $$quote($$BASEDIR/src/appsettings.h) \
                 $$quote($$BASEDIR/src/playerlauncher.h) \
                 $$quote($$BASEDIR/src/ytarraydatamodel.h) \
                 $$quote($$BASEDIR/src/ytvideomanager.h)
    }
}

INCLUDEPATH +=  $$quote($$BASEDIR/src)

CONFIG += precompile_header

PRECOMPILED_HEADER =  $$quote($$BASEDIR/precompiled.h)

lupdate_inclusion {
    SOURCES +=  $$quote($$BASEDIR/../src/*.c) \
             $$quote($$BASEDIR/../src/*.c++) \
             $$quote($$BASEDIR/../src/*.cc) \
             $$quote($$BASEDIR/../src/*.cpp) \
             $$quote($$BASEDIR/../src/*.cxx) \
             $$quote($$BASEDIR/../assets/*.qml) \
             $$quote($$BASEDIR/../assets/*.js) \
             $$quote($$BASEDIR/../assets/*.qs)

    HEADERS +=  $$quote($$BASEDIR/../src/*.h) \
             $$quote($$BASEDIR/../src/*.h++) \
             $$quote($$BASEDIR/../src/*.hh) \
             $$quote($$BASEDIR/../src/*.hpp) \
             $$quote($$BASEDIR/../src/*.hxx)
}

TRANSLATIONS =  $$quote($${TARGET}_ru.ts) \
         $$quote($${TARGET}.ts)
