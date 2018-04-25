DESTDIR = $$PWD/../bin
TEMPLATE = app
CONFIG += console

#LIBS += -L$$PWD/../lpsolve -llpsolve55

build_pass:CONFIG(debug, debug|release){
    OBJECTS_DIR = Debug
    LIBS += -L$$PWD/../thirdparty/boost/debug
    LIBS += -L$$PWD/../bin -ldag-lib_d
    LIBS += -ldag-lib_d
}else:build_pass:CONFIG(release, debug|release){
    OBJECTS_DIR = Release
    LIBS += -L$$PWD/../thirdparty/boost/release
    LIBS += -L$$PWD/../bin -ldag-lib
}

win32-msvc* : QMAKE_LFLAGS += /INCREMENTAL:NO

INCLUDEPATH += $$PWD
INCLUDEPATH += $(BOOST_HOME)

DEPENDPATH += -L$$PWD/dag-lib

#mylib.path = $$PWD/../bin
#mylib.files += $$files($$PWD/../lpsolve/*.dll)
#message("found files for mylib: "$$mylib.files)
#win32: mylib.files ~= s,/,\\,g
#INSTALLS += mylib
