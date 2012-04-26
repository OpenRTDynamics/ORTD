/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <QtPlugin>
#include "qpl_designer_plugin.h"

#include "scope_plugin.h"
#include "led_plugin.h"

QPLWidgetsPlugin::QPLWidgetsPlugin()
{
//  m_pluginList.push_back(new TestPlugin(NULL));
    m_pluginList.push_back(new ScopePlugin(NULL));
    m_pluginList.push_back(new LedPlugin(NULL));
 // m_pluginList.push_back(new WheelPlugin(NULL)); 
 // m_pluginList.push_back(new MnemonicBoxPlugin(NULL)); 
}

QPLWidgetsPlugin::~QPLWidgetsPlugin()
{
  QList<QDesignerCustomWidgetInterface *>::Iterator i = m_pluginList.begin();
  while (i != m_pluginList.end())
  delete *i++;
  m_pluginList.clear();
}

QList<QDesignerCustomWidgetInterface *>
QPLWidgetsPlugin:: customWidgets() const
{
  return m_pluginList;
}

#ifndef SINGLEPLUGINS
Q_EXPORT_PLUGIN(QPLWidgetsPlugin)
#endif
