/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicNewEditableWellPathFeature.h"

#include "RiaColorTables.h"

#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEditableWellPathFeature, "RicNewEditableWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEditableWellPathFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPath*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        if ( objects.size() > 0 )
        {
            return true;
        }
    }
    {
        std::vector<RimWellPathCollection*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        if ( objects.size() > 0 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    if ( project && RimProject::current()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            std::vector<RimWellPath*> newWellPaths;
            auto                      newModeledWellPath = new RimModeledWellPath();
            newWellPaths.push_back( newModeledWellPath );

            newModeledWellPath->setUnitSystem( project->commonUnitSystemForAllCases() );

            size_t modelledWellpathCount = wellPathCollection->modelledWellPathCount();

            newWellPaths.back()->setName( "Well-" + QString::number( modelledWellpathCount + 1 ) );
            newModeledWellPath->setWellPathColor( RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( modelledWellpathCount ) );

            wellPathCollection->addWellPaths( newWellPaths );
            wellPathCollection->uiCapability()->updateConnectedEditors();

            newModeledWellPath->geometryDefinition()->enableTargetPointPicking( true );

            project->scheduleCreateDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath->geometryDefinition() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Path" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
