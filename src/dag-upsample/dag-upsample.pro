! include( ../app-shared.pri ) {
    error( "Could not find the app-shared.pri file!" )
}

TARGET = dag-upsample
TEMPLATE = app

SOURCES += \
    main.cpp \ 
    DAGMergeUp.cpp

HEADERS += \
    DAGMergeup.h


