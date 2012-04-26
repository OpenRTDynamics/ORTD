# -*- mode: sh -*- ################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
###################################################################

QPL_ROOT = ../..

include( $${QPL_ROOT}/qplconfig.pri )

SUFFIX_STR =
    CONFIG(debug, debug|release) {
        SUFFIX_STR = $${DEBUG_SUFFIX}
    }
    else {
        SUFFIX_STR = $${RELEASE_SUFFIX}
    }

TEMPLATE     = app

MOC_DIR      = moc
INCLUDEPATH += $${QPL_ROOT}/src
#INCLUDEPATH += $${QPL_ROOT}/src/qwt/src
INCLUDEPATH += /usr/include/qwt
DEPENDPATH  += $${QPL_ROOT}/src
OBJECTS_DIR  = obj$${SUFFIX_STR}
DESTDIR      = $${QPL_ROOT}/examples/bin$${SUFFIX_STR}

QPLLIB       = qpl$${SUFFIX_STR}

win32 {
    contains(CONFIG, QplDll) {
        DEFINES    += QT_DLL QPL_DLL
        QPLLIB = $${QPLLIB}$${VER_MAJ}
    }

    win32-msvc:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc.net:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2002:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2003:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2005:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2008:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-g++:LIBS   += -L$${QWT_ROOT}/lib -l$${QWTLIB}
}
else {
    LIBS        += -L$${QPL_ROOT}/lib -l$${QPLLIB}
#    LIBS	+= -L$${QPL_ROOT}/src/qwt/lib -lqwt
  #   LIBS        += -lqwt

}
