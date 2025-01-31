/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimViewLinkerCollection.h"

#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimViewLinkerCollection, "RimViewLinkerCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinkerCollection::RimViewLinkerCollection()
{
    CAF_PDM_InitObject( "Linked Views", ":/LinkView.svg" );

    CAF_PDM_InitField( &isActive, "Active", true, "Active" );
    isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &viewLinker, "ViewLinkers", "View Linkers" );
    viewLinker.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinkerCollection::~RimViewLinkerCollection()
{
    if ( viewLinker() ) delete viewLinker();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    RimViewLinker* childObject = viewLinker();
    if ( childObject )
    {
        uiTreeOrdering.add( childObject );
        childObject->addViewControllers( uiTreeOrdering );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &isActive == changedField )
    {
        if ( viewLinker() )
        {
            if ( !isActive )
            {
                viewLinker()->applyCellFilterCollectionByUserChoice();
            }

            viewLinker()->updateDependentViews();
            viewLinker()->updateWindowTitles();
        }
    }

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::initAfterRead()
{
    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    uiCapability()->updateConnectedEditors();

    RimProject* project = RimProject::current();
    if ( project )
    {
        // Update visibility of top level Linked Views item in the project tree
        // Not visible if no views are linked
        project->uiCapability()->updateConnectedEditors();
    }
}
