/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicCreateGridStatisticsPlotFeature.h"

#include "RiaGuiApplication.h"

#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGridStatisticsPlot.h"
#include "RimGridStatisticsPlotCollection.h"
#include "RimMainPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateGridStatisticsPlotFeature, "RicCreateGridStatisticsPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateGridStatisticsPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridStatisticsPlotFeature::onActionTriggered( bool isChecked )
{
    RimGridStatisticsPlotCollection* collection = RimMainPlotCollection::current()->gridStatisticsPlotCollection();

    RimGridStatisticsPlot* plot = new RimGridStatisticsPlot();

    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    if ( activeView ) plot->setPropertiesFromView( activeView );

    plot->zoomAll();
    plot->updateConnectedEditors();
    plot->setAsPlotMdiWindow();
    plot->loadDataAndUpdate();

    collection->addGridStatisticsPlot( plot );
    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindowTools::onObjectAppended( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateGridStatisticsPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Grid Statistics Plot" );
    actionToSetup->setIcon( QIcon( ":/statistics.png" ) );
}
