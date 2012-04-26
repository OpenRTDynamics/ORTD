include( qplconfig.pri )

TEMPLATE = subdirs
CONFIG   += ordered

SUBDIRS = \
    src 

contains(CONFIG, QplDesigner ) {
    SUBDIRS += designer 
}

contains(CONFIG, QplExamples ) {
    SUBDIRS += examples 
}
