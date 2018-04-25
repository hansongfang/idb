DESTDIR = $$PWD/../../bin
TEMPLATE = app
CONFIG += console
TARGET = dag-viewer

build_pass:CONFIG(debug, debug|release){
    OBJECTS_DIR = Debug
    LIBS += -L$$PWD/../../thirdparty/boost/debug
    LIBS += -L$$PWD/../../bin -ldag-lib_d
}else:build_pass:CONFIG(release, debug|release){
    OBJECTS_DIR = Release
    LIBS += -L$$PWD/../../thirdparty/boost/release
    LIBS += -L$$PWD/../../bin -ldag-lib
}

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $(BOOST_HOME)
INCLUDEPATH += $$PWD/../../thirdparty/glm \
               $$PWD/../../thirdparty/glew/include\
               $$PWD/../../thirdparty/glfw/include \
               $$PWD/../../thirdparty/freeImage \

LIBS += -L$$PWD/../../thirdparty/glew/lib -lglew32 \
        -L$$PWD/../../thirdparty/glfw/lib -lglfw3dll \
        -L$$PWD/../../thirdparty/freeImage -lFreeImage

LIBS += -lopengl32
DEPENDPATH += -L$$PWD/../dag-lib

win32-msvc* : QMAKE_LFLAGS += /INCREMENTAL:NO

SOURCES += \
    main.cpp \
    viewSample.cpp \
    main.cpp \
    GLWindow.cpp \
    Camera.cpp


HEADERS += \
    viewSample.h \
    GLWindow.h \
    Camera.h


