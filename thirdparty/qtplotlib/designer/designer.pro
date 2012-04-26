QPL_ROOT = ..
include ( $${QPL_ROOT}/qplconfig.pri )
contains(CONFIG, QplDesigner) { 
    CONFIG += warn_on
    SUFFIX_STR = 
    CONFIG(debug, debug|release):SUFFIX_STR = $${DEBUG_SUFFIX}
    else:SUFFIX_STR = $${RELEASE_SUFFIX}
    TEMPLATE = lib
    MOC_DIR = moc
    OBJECTS_DIR = obj$${SUFFIX_STR}
    DESTDIR = plugins/designer
    INCLUDEPATH += $${QPL_ROOT}/src
  #  INCLUDEPATH += $${QPL_ROOT}/src/qwt/src
    DEPENDPATH += $${QPL_ROOT}/src
    LIBNAME = qpl$${SUFFIX_STR}
    contains(CONFIG, QplDll):win32 { 
        DEFINES += QT_DLL \
            QPL_DLL
        LIBNAME = $${LIBNAME}$${VER_MAJ}
    }
    !contains(CONFIG, QplPlot):DEFINES += NO_QPL_PLOT
    unix:LIBS += -L$${QPL_ROOT}/lib \
        -l$${LIBNAME}
    
    # unix:LIBS      += -L$${QPL_ROOT}/src/qwt/lib -lqwt
#    unix:LIBS += -lqwt
    win32-msvc:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-msvc.net:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-msvc2002:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-msvc2003:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-msvc2005:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-msvc2008:LIBS += $${QPL_ROOT}/lib/$${LIBNAME}.lib
    win32-g++:LIBS += -L$${QPL_ROOT}/lib \
        -l$${LIBNAME}
    
    # isEmpty(QT_VERSION) does not work with Qt-4.1.0/MinGW
    # Qt 4
    TARGET = qpl_designer_plugin$${SUFFIX_STR}
    CONFIG += qt \
        designer \
        plugin
    RCC_DIR = resources
    HEADERS += qpl_designer_plugin.h \
        scope_plugin.h \
        led_plugin.h
    SOURCES += qpl_designer_plugin.cpp \
        scope_plugin.cpp \
        led_plugin.cpp
    RESOURCES += qpl_designer_plugin.qrc
    target.path = $$[QT_INSTALL_PLUGINS]/designer
    INSTALLS += target
}
else:TEMPLATE = subdirs # do nothing
