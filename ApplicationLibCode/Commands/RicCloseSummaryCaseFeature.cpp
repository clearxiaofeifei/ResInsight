/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicCloseSummaryCaseFeature.h"

#include "RiaGuiApplication.h"
#include "RiaSummaryTools.h"

#include "RicDeleteItemFeature.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimWellLogPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafAsyncObjectDeleter.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCloseSummaryCaseFeature, "RicCloseSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Close Summary Case" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::deleteSummaryCases( std::vector<RimSummaryCase*>& cases )
{
    RimSummaryMultiPlotCollection* summaryPlotColl           = RiaSummaryTools::summaryMultiPlotCollection();
    RimSummaryCaseMainCollection*  summaryCaseMainCollection = RiaSummaryTools::summaryCaseMainCollection();

    std::set<RimSummaryMultiPlot*> plotsToUpdate;
    std::set<RimWellLogPlot*>      wellLogPlotsToDelete;

    for ( RimSummaryCase* summaryCase : cases )
    {
        for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
        {
            for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
            {
                summaryPlot->deleteCurvesAssosiatedWithCase( summaryCase );
            }
            plotsToUpdate.insert( multiPlot );
        }

        std::vector<caf::PdmObjectHandle*> referringObjects = summaryCase->objectsWithReferringPtrFields();

        for ( auto object : referringObjects )
        {
            if ( !object ) continue;

            RimWellLogPlot* wellLogPlot = object->firstAncestorOrThisOfType<RimWellLogPlot>();
            if ( wellLogPlot ) wellLogPlotsToDelete.insert( wellLogPlot );
        }
    }

    for ( auto wellLogPlot : wellLogPlotsToDelete )
    {
        RicDeleteItemFeature::deleteObject( wellLogPlot );
    }

    summaryCaseMainCollection->removeCases( cases );

    for ( auto plot : plotsToUpdate )
    {
        plot->updateConnectedEditors();
        plot->updatePlots();
    }

    summaryCaseMainCollection->updateAllRequiredEditors();

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();

    caf::AsyncPdmObjectVectorDeleter<RimSummaryCase> summaryCaseDeleter( cases );
    CAF_ASSERT( cases.empty() ); // vector should be empty immediately.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseFeature::isCommandEnabled()
{
    std::vector<RimSummaryCase*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() == 0 )
    {
        return false;
    }

    for ( RimSummaryCase* summaryCase : selection )
    {
        if ( summaryCase->isObservedData() )
        {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCase*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );
    CVF_ASSERT( selection.size() > 0 );

    RicCloseSummaryCaseFeature::deleteSummaryCases( selection );
}
