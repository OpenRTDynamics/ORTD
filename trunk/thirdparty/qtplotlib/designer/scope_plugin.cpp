/***************************************************************************
 *   Copyright (C) 2006-2008 by Tomasz Ziobrowski                          *
 *   http://www.3electrons.com                                             *
 *   e-mail: t.ziobrowski@3electrons.com                                   *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

    #include "scope.h"
    #include "scope_plugin.h"  
    /*------------------------------------------------------------------------------------------------
     *			     			SCOPE    
     *------------------------------------------------------------------------------------------------*/

    ScopePlugin::ScopePlugin(QObject *parent)
        : QObject(parent)
    {
        initialized = false;
    }

    void ScopePlugin::initialize(QDesignerFormEditorInterface * /*core*/)
    {
        if (initialized)
            return;

        initialized = true;
    }

    bool ScopePlugin::isInitialized() const
    {
        return initialized;
    }

    QWidget *ScopePlugin::createWidget(QWidget *parent)
    {
        return new QPL_Scope(parent);
    }

    QString ScopePlugin::name() const
    {
        return QLatin1String("QPL_Scope");
    }

    QString ScopePlugin::group() const
    {
        return QLatin1String("QPL");
    }

    QIcon ScopePlugin::icon() const
    {
        return QIcon();
    }

    QString ScopePlugin::toolTip() const
    {
        return QString();
    }

    QString ScopePlugin::whatsThis() const
    {
        return QString();
    }

    bool ScopePlugin::isContainer() const
    {
        return false;
    }

    QString ScopePlugin::domXml() const
    {
        return QLatin1String("<widget class=\"QPL_Scope\" name=\"scope\">\n"
                             " <property name=\"geometry\">\n"
                             "  <rect>\n"
                             "   <x>0</x>\n"
                             "   <y>0</y>\n"
                             "   <width>100</width>\n"
                             "   <height>100</height>\n"
                             "  </rect>\n"
                             " </property>\n"
                             " <property name=\"toolTip\" >\n"
                             "  <string>Plotting</string>\n"
                             " </property>\n"
                             " <property name=\"whatsThis\" >\n"
                             "  <string>The Scope plots data</string>\n"
                             " </property>\n"
                             "</widget>\n");
    }

    QString ScopePlugin::includeFile() const
    {
        return QLatin1String("scope.h");
    }

    QString ScopePlugin::codeTemplate() const
    {
        return QString();
    }

#ifdef SINGLEPLUGINS
    Q_EXPORT_PLUGIN(ScopePlugin)
#endif
