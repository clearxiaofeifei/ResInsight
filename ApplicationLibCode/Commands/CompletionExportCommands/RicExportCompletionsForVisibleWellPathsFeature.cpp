/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportCompletionsForVisibleWellPathsFeature.h"

#include "RiaLogging.h"

#include "RicWellPathExportCompletionDataFeature.h"

#include "RimProject.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicExportCompletionsForVisibleWellPathsFeature, "RicExportCompletionsForVisibleWellPathsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsForVisibleWellPathsFeature::isCommandEnabled()
{
    bool                         foundWellPathCollection = false;
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );
    for ( caf::PdmObject* object : selectedObjects )
    {
        RimWellPathCollection* wellPathCollection = object->firstAncestorOrThisOfType<RimWellPathCollection>();
        if ( wellPathCollection )
        {
            foundWellPathCollection = true;
            break;
        }
    }

    if ( !foundWellPathCollection )
    {
        return false;
    }

    std::vector<RimWellPath*> wellPaths = visibleWellPaths();

    if ( wellPaths.empty() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleWellPathsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths = visibleWellPaths();
    if ( wellPaths.empty() ) return RiaLogging::warning( "No visible well paths found, no data exported." );

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Export Completion Data for Visible Well Paths";

    RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions( dialogTitle, wellPaths, simWells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleWellPathsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Completion Data for Visible Well Paths" );
    actionToSetup->setIcon( QIcon( ":/ExportCompletionsSymbol16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportCompletionsForVisibleWellPathsFeature::visibleWellPaths()
{
    std::vector<RimWellPath*> wellPaths;

    {
        auto measurementColl = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellMeasurementCollection>();
        if ( measurementColl ) return wellPaths;
    }

    {
        std::vector<RimWellPathCollection*> wellPathCollections;
        caf::SelectionManager::instance()->objectsByType( &wellPathCollections );

        if ( wellPathCollections.empty() )
        {
            std::vector<RimWellPath*> selectedWellPaths;
            caf::SelectionManager::instance()->objectsByType( &selectedWellPaths );

            if ( !selectedWellPaths.empty() )
            {
                RimWellPathCollection* parent = selectedWellPaths[0]->firstAncestorOrThisOfType<RimWellPathCollection>();
                if ( parent )
                {
                    wellPathCollections.push_back( parent );
                }
            }
        }

        if ( !wellPathCollections.empty() )
        {
            for ( auto wellPathCollection : wellPathCollections )
            {
                for ( const auto& wellPath : wellPathCollection->allWellPaths() )
                {
                    if ( wellPath->showWellPath() )
                    {
                        wellPaths.push_back( wellPath );
                    }
                }
            }
        }
        else
        {
            // No well path or well path collection selected

            auto allWellPaths = RimProject::current()->allWellPaths();
            for ( const auto& w : allWellPaths )
            {
                if ( w->showWellPath() )
                {
                    wellPaths.push_back( w );
                }
            }
        }
    }

    return wellPaths;
}
