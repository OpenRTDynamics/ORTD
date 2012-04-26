# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
# #############################################
# qmake project file for building the qwt libraries
QPL_ROOT = ..
include( $${QPL_ROOT}/qplconfig.pri )
SUFFIX_STR = 
CONFIG(debug, debug|release):SUFFIX_STR = $${DEBUG_SUFFIX}
else:SUFFIX_STR = $${RELEASE_SUFFIX}
TARGET = qpl$${SUFFIX_STR}
TEMPLATE = lib
MOC_DIR = moc
OBJECTS_DIR = obj$${SUFFIX_STR}
DESTDIR = $${QPL_ROOT}/lib
contains(CONFIG, QplDll ):CONFIG += dll
else:CONFIG += staticlib
win32:QplDll:DEFINES += QT_DLL \
    QPL_DLL \
    QPL_MAKEDLL
symbian:QplDll:DEFINES += QT_DLL \
    QPL_DLL \
    QPL_MAKEDLL


SOURCES += scope.cpp \
    scope_trace.cpp \
    scope_data.cpp \
    data2disk.cpp \
    dial_qwt.cpp \
    lcd_qlabel.cpp \
    led.cpp \
    led_data.cpp \
    matrix_plot.cpp \
    matrix_view.cpp \
    thermo_qwt.cpp \
    xyplot.cpp \
    xyplot_trace.cpp
HEADERS += scope.h \
    qtplot_global.h \
    scope_data.h \
    scope_trace.h \
    data2disk.h \
    dial_qwt.h \
    xyplot_trace.h \
    xyplot.h \
    thermo_qwt.h \
    matrix_view.h \
    matrix_plot.h \
    led.h \
    led_data.h \
    lcd_qlabel.h

# Install directives
headers.files = $$HEADERS
doc.files = $${QPL_ROOT}/doc/html \
    $${QPL_ROOT}/doc/qpl.qch
unix:doc.files += $${QPL_ROOT}/doc/man
INSTALLS = target \
    headers \
    doc
