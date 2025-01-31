/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimStimPlanModelPlot.h"

#include "RiaDefines.h"
#include "RiaStimPlanModelDefines.h"

#include "Riu3DMainWindowTools.h"

#include "RimEclipseCase.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCurve.h"
#include "RimStimPlanModelPropertyCurve.h"
#include "RimTools.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"

#include "cafPdmBase.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiToolButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimStimPlanModelPlot, "StimPlanModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlot::RimStimPlanModelPlot()
{
    CAF_PDM_InitScriptableObject( "StimPlan Model Plot", "", "", "A fracture model plot" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModel, "StimPlanModel", "StimPlan Model" );
    m_stimPlanModel.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_editStimPlanModel, "EditModel", false, "Edit" );
    m_editStimPlanModel.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_editStimPlanModel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "Case" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step" );

    setLegendsVisible( true );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlot::setStimPlanModel( RimStimPlanModel* stimPlanModel )
{
    m_stimPlanModel = stimPlanModel;
    m_eclipseCase   = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::UNDEFINED );
    m_timeStep      = stimPlanModel->timeStep();

    m_nameConfig->setCustomName( stimPlanModel->name() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel* RimStimPlanModelPlot::stimPlanModel()
{
    return m_stimPlanModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_stimPlanModel, { true, 2, 1 } );
    uiOrdering.add( &m_editStimPlanModel, { false, 1, 0 } );
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.add( &m_timeStep );

    caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis" );
    RimDepthTrackPlot::uiOrderingForDepthAxis( uiConfigName, *depthGroup );

    caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    RimDepthTrackPlot::uiOrderingForAutoName( uiConfigName, *titleGroup );

    RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanModelPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_stimPlanModel && m_stimPlanModel )
    {
        // The user is not allowed to change this field, but option box looks good
        options.push_back( caf::PdmOptionItemInfo( m_stimPlanModel->name(), m_stimPlanModel ) );
    }
    else if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_eclipseCase(), &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimDepthTrackPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( m_stimPlanModel )
    {
        if ( changedField == &m_eclipseCase || changedField == &m_timeStep )
        {
            m_stimPlanModel->setEclipseCaseAndTimeStep( m_eclipseCase(), m_timeStep() );
        }
        else if ( changedField == &m_editStimPlanModel )
        {
            m_editStimPlanModel = false;
            if ( m_stimPlanModel != nullptr )
            {
                Riu3DMainWindowTools::selectAsCurrentItem( m_stimPlanModel() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlot::onLoadDataAndUpdate()
{
    if ( stimPlanModel() != nullptr )
    {
        // Update eclipse case and time step
        m_eclipseCase = stimPlanModel()->eclipseCaseForProperty( RiaDefines::CurveProperty::UNDEFINED );
        m_timeStep    = stimPlanModel()->timeStep();
        updateConnectedEditors();
    }

    RimDepthTrackPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPlot::applyDataSource()
{
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RimStimPlanModelPlot::findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RimStimPlanModelPropertyCurve*> curves = descendantsIncludingThisOfType<RimStimPlanModelPropertyCurve>();

    for ( RimStimPlanModelPropertyCurve* curve : curves )
    {
        if ( curve->curveProperty() == curveProperty )
        {
            return dynamic_cast<RimWellLogExtractionCurve*>( curve );
        }
    }

    return nullptr;
}
