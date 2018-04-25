! include( ../app-shared.pri ) {
    error( "Could not find the app-shared.pri file!" )
}

TARGET = dag-merger
TEMPLATE = app

SOURCES += \
    main.cpp \ 
    DAGMaker.cpp \
    BufferMaker.cpp \
    DAGFanVCache.cpp

HEADERS += \
    DAGMaker.h \
    BufferMaker.h \
    DAGFanVCache.h


