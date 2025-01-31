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

#include "RicPasteTimeHistoryCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "RimGridTimeHistoryCurve.h"
#include "RimSummaryPlot.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteTimeHistoryCurveFeature, "RicPasteTimeHistoryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteTimeHistoryCurveFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    if ( !destinationObject )
    {
        return false;
    }

    auto summaryPlot = destinationObject->firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( !summaryPlot )
    {
        return false;
    }

    return RicPasteTimeHistoryCurveFeature::timeHistoryCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteTimeHistoryCurveFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    CVF_ASSERT( destinationObject );

    auto summaryPlot = destinationObject->firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( !summaryPlot )
    {
        return;
    }

    std::vector<caf::PdmPointer<RimGridTimeHistoryCurve>> sourceObjects = RicPasteTimeHistoryCurveFeature::timeHistoryCurves();

    for ( size_t i = 0; i < sourceObjects.size(); i++ )
    {
        RimGridTimeHistoryCurve* newObject = dynamic_cast<RimGridTimeHistoryCurve*>(
            sourceObjects[i]->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
        CVF_ASSERT( newObject );

        summaryPlot->addGridTimeHistoryCurve( newObject );

        // Resolve references after object has been inserted into the project data model
        newObject->resolveReferencesRecursively();

        // If source curve is part of a curve filter, resolve of references to the summary case does not
        // work when pasting the new curve into a plot. Must set summary case manually.
        // newObject->setSummaryCase(sourceObjects[i]->summaryCase());

        newObject->initAfterReadRecursively();

        newObject->loadDataAndUpdate( true );
        newObject->updateConnectedEditors();

        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteTimeHistoryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Time History Curve" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimGridTimeHistoryCurve>> RicPasteTimeHistoryCurveFeature::timeHistoryCurves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimGridTimeHistoryCurve>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
