######################################################################
# Install paths
######################################################################

VER_MAJ      = 0
VER_MIN      = 1
VER_PAT      = 0
VERSION      = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

unix {
    INSTALLBASE    = /usr/local/
}

win32 {
    INSTALLBASE    = C:/Qwt-$$VERSION-svn
}

target.path    = $$INSTALLBASE/lib
headers.path   = $$INSTALLBASE/include
doc.path       = $$INSTALLBASE/doc

######################################################################
# qwt dir
######################################################################

# INCLUDEPATH += qwt/src
unix:INCLUDEPATH += /usr/include/qwt-qt4

# LIBS += -L qwt/lib
unix:LIBS += -lqwt-qt4

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     # Also for Qtopia Core!
CONFIG           += warn_on
CONFIG           += thread
CONFIG           += silent


######################################################################
# release/debug mode
# If you want to build both DEBUG_SUFFIX and RELEASE_SUFFIX
# have to differ to avoid, that they overwrite each other.
######################################################################

VVERSION = $$[QT_VERSION]
isEmpty(VVERSION) {

    # Qt 3
    CONFIG           += debug     # release/debug
}
else {
    # Qt 4
    win32 {
        # On Windows you can't mix release and debug libraries.
        # The designer is built in release mode. If you like to use it
        # you need a release version. For your own application development you
        # might need a debug version. 
        # Enable debug_and_release + build_all if you want to build both.

        CONFIG           += debug     # release/debug/debug_and_release
        #CONFIG           += debug_and_release
        #CONFIG           += build_all
    }
    else {
        CONFIG           += debug     # release/debug
    }
}

######################################################################
# If you want to have different names for the debug and release 
# versions you can add a suffix rule below.
######################################################################

DEBUG_SUFFIX        = 
RELEASE_SUFFIX      = 

win32 {
    DEBUG_SUFFIX      = d
}

######################################################################
# Build the static/shared libraries.
# If QwtDll is enabled, a shared library is built, otherwise
# it will be a static library.
######################################################################

CONFIG           += QplDll

######################################################################
# QwtPlot enables all classes, that are needed to use the QwtPlot 
# widget. 
######################################################################

CONFIG       += QplPlot



######################################################################
# If you want to build the Qwt designer plugin, 
# enable the line below.
# Otherwise you have to build it from the designer directory.
######################################################################

CONFIG     += QplDesigner

######################################################################
# If you want to auto build the examples, enable the line below
# Otherwise you have to build them from the examples directory.
######################################################################

#CONFIG     += QplExamples
