/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicNewElasticPropertyScalingFeature.h"

#include "RimElasticProperties.h"
#include "RimElasticPropertyScaling.h"
#include "RimElasticPropertyScalingCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewElasticPropertyScalingFeature, "RicNewElasticPropertyScalingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewElasticPropertyScalingFeature::onActionTriggered( bool isChecked )
{
    RimElasticProperties* elasticProperties = caf::SelectionManager::instance()->selectedItemOfType<RimElasticProperties>();
    if ( !elasticProperties ) return;

    RimElasticPropertyScalingCollection* scalingColl = elasticProperties->scalingCollection();
    if ( !scalingColl ) return;

    RimElasticPropertyScaling* elasticPropertyScaling = new RimElasticPropertyScaling;
    elasticPropertyScaling->ensureDefaultFormationAndFacies();

    scalingColl->addElasticPropertyScaling( elasticPropertyScaling );

    scalingColl->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( elasticPropertyScaling );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewElasticPropertyScalingFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Property Scaling" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewElasticPropertyScalingFeature::isCommandEnabled()
{
    return true;
}
