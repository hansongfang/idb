TARGET = dag-lib

DESTDIR = $$PWD/../../bin
TEMPLATE = lib

CONFIG += shared
CONFIG += lib
DEFINES += DEF_TRIDAG_LIB

build_pass:CONFIG(debug, debug|release):{
    TARGET = $$join(TARGET,,,_d)
    OBJECTS_DIR = Debug
    LIBS += -L$$PWD/../../thirdparty/boost/debug
}else:build_pass:CONFIG(release, debug|release):{
    OBJECTS_DIR = Release
    LIBS += -L$$PWD/../../thirdparty/boost/release
}
win32-msvc* : QMAKE_LFLAGS += /INCREMENTAL:NO

INCLUDEPATH += $$PWD/../../
INCLUDEPATH += $(BOOST_HOME)

msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

SOURCES += \
    rply.c \
    appcommon.cpp \
    DAG.cpp \
    DAGCenter.cpp \
    DagGraph.cpp \
    GraphCommon.cpp \
    Occluder.cpp \
    ViewBase.cpp \
    ViewFacet.cpp \
    ViewPoint.cpp \
    ModelBase.cpp \
    DagMerger.cpp \
    SampledTriangle.cpp \
    InitViewModel.cpp
	
HEADERS += \
    rply.h \
    appcommon.h \
    DAG.h \
    DAGCenter.h \
    DagGraph.h \
    GraphCommon.h \
    Occluder.h \
    ViewBase.h \
    ViewFacet.h \
    ViewPoint.h \
    ModelBase.h \
    DagMerger.h \
    SampledTriangle.h \
    SFMath.h \
    SFVector.h \
    Log.h \
    DLL.h \
    InitViewModel.h
