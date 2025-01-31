/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewSimWellFractureFeature.h"

#include "RicFractureNameGenerator.h"

#include "RiaApplication.h"
#include "RigEclipseCaseData.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimStimPlanColors.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSimWellFractureFeature, "RicNewSimWellFractureFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::onActionTriggered( bool isChecked )
{
    RimProject* proj = RimProject::current();
    if ( proj->allFractureTemplates().empty() ) return;

    auto objHandle = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>();
    if ( !objHandle ) return;

    auto eclipseWell = objHandle->firstAncestorOrThisOfType<RimSimWellInView>();

    RimSimWellFracture* fracture = new RimSimWellFracture();
    if ( eclipseWell->simwellFractureCollection()->simwellFractures.empty() )
    {
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
        if ( activeView )
        {
            activeView->fractureColors()->setDefaultResultName();
        }
    }

    eclipseWell->simwellFractureCollection()->simwellFractures.push_back( fracture );

    auto oilfield = objHandle->firstAncestorOrThisOfType<RimOilField>();
    if ( !oilfield ) return;

    fracture->setName( RicFractureNameGenerator::nameForNewFracture() );

    auto unitSet = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    {
        auto eclipseCase = objHandle->firstAncestorOrThisOfType<RimEclipseResultCase>();
        if ( eclipseCase )
        {
            unitSet = eclipseCase->eclipseCaseData()->unitsType();
        }
        fracture->setFractureUnit( unitSet );
    }

    RimFractureTemplate* fracDef = oilfield->fractureDefinitionCollection()->firstFractureOfUnit( unitSet );
    fracture->setFractureTemplate( fracDef );

    fracture->updateFracturePositionFromLocation();

    eclipseWell->updateConnectedEditors();

    auto eclipseCase = objHandle->firstAncestorOrThisOfType<RimEclipseCase>();
    if ( eclipseCase )
    {
        proj->reloadCompletionTypeResultsForEclipseCase( eclipseCase );
        eclipseWell->simwellFractureCollection()->updateConnectedEditors();
    }
    Riu3DMainWindowTools::selectAsCurrentItem( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "New Fracture" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureFeature::isCommandEnabled()
{
    RimProject* proj = RimProject::current();
    if ( proj->allFractureTemplates().empty() ) return false;

    auto objHandle = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>();
    if ( !objHandle ) return false;

    auto simWell = objHandle->firstAncestorOrThisOfType<RimSimWellInView>();
    if ( simWell )
    {
        return true;
    }

    return false;
}
