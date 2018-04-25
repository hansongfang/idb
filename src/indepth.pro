TEMPLATE = subdirs

# build project in order
CONFIG += ordered

SUBDIRS = \
    dag-lib \
    dag-merger \
    dag-upsample \
    dag-viewer

dag-merger.depends = \
    dag-lib
