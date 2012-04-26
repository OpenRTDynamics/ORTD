/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QPL_DESIGNER_PLUGIN_H
#define QPL_DESIGNER_PLUGIN_H

#include <qglobal.h>

#undef SINGLEPLUGINS

#include <QDesignerCustomWidgetCollectionInterface>
#include <QList>

class QPLWidgetsPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES (QDesignerCustomWidgetCollectionInterface)

  QList<QDesignerCustomWidgetInterface *> m_pluginList;

  public:
     QPLWidgetsPlugin();
    ~QPLWidgetsPlugin();

    QList<QDesignerCustomWidgetInterface *> customWidgets () const;

}; // AnalogWidgetPlugin



#endif
