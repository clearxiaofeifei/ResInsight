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

#include "RimFractureTemplateCollection.h"

#include "RiaLogging.h"

#include "RigStatisticsMath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimMeshFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimFractureTemplateCollection, "FractureTemplateCollection", "FractureDefinitionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::RimFractureTemplateCollection()
{
    CAF_PDM_InitScriptableObject( "Fracture Templates", ":/FractureTemplates16x16.png" );

    CAF_PDM_InitField( &m_defaultUnitsForFracTemplates,
                       "DefaultUnitForTemplates",
                       caf::AppEnum<RiaDefines::EclipseUnitSystem>( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                       "Default unit system for fracture templates" );

    CAF_PDM_InitFieldNoDefault( &m_fractureDefinitions, "FractureDefinitions", "" );
    m_fractureDefinitions.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_nextValidFractureTemplateId, "NextValidFractureTemplateId", 0, "" );
    m_nextValidFractureTemplateId.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::~RimFractureTemplateCollection()
{
    m_fractureDefinitions.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFractureTemplateCollection::fractureTemplate( int id ) const
{
    for ( const auto& templ : m_fractureDefinitions )
    {
        if ( templ->id() == id ) return templ;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplate*> RimFractureTemplateCollection::fractureTemplates() const
{
    std::vector<RimFractureTemplate*> templates;
    for ( auto& templ : m_fractureDefinitions )
    {
        templates.push_back( templ );
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate* RimFractureTemplateCollection::addDefaultEllipseTemplate()
{
    RimEllipseFractureTemplate* ellipseFractureTemplate = new RimEllipseFractureTemplate();

    addFractureTemplate( ellipseFractureTemplate );

    ellipseFractureTemplate->setName( "Ellipse Fracture Template" );
    ellipseFractureTemplate->setUnitSystem( defaultUnitSystemType() );
    ellipseFractureTemplate->setDefaultValuesFromUnit();

    return ellipseFractureTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::addFractureTemplate( RimFractureTemplate* templ )
{
    templ->setId( nextFractureTemplateId() );
    m_fractureDefinitions.push_back( templ );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimFractureTemplateCollection::defaultUnitSystemType() const
{
    return m_defaultUnitsForFracTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::setDefaultUnitSystemBasedOnLoadedCases()
{
    RimProject* proj = RimProject::current();

    auto commonUnitSystem = proj->commonUnitSystemForAllCases();
    if ( commonUnitSystem != RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
    {
        m_defaultUnitsForFracTemplates = commonUnitSystem;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFractureTemplateCollection::firstFractureOfUnit( RiaDefines::EclipseUnitSystem unitSet ) const
{
    for ( RimFractureTemplate* f : m_fractureDefinitions() )
    {
        if ( f->fractureTemplateUnit() == unitSet )
        {
            return f;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimFractureTemplateCollection::resultNamesAndUnits() const
{
    std::set<std::pair<QString, QString>> nameSet;

    for ( const RimFractureTemplate* f : m_fractureDefinitions() )
    {
        std::vector<std::pair<QString, QString>> namesAndUnits = f->uiResultNamesWithUnit();

        for ( const auto& nameAndUnit : namesAndUnits )
        {
            nameSet.insert( nameAndUnit );
        }
    }

    std::vector<std::pair<QString, QString>> names( nameSet.begin(), nameSet.end() );

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::computeMinMax( const QString& uiResultName,
                                                   const QString& unit,
                                                   double*        minValue,
                                                   double*        maxValue,
                                                   double*        posClosestToZero,
                                                   double*        negClosestToZero ) const
{
    MinMaxAccumulator minMaxAccumulator;
    PosNegAccumulator posNegAccumulator;

    for ( const RimFractureTemplate* f : m_fractureDefinitions() )
    {
        if ( f )
        {
            f->appendDataToResultStatistics( uiResultName, unit, minMaxAccumulator, posNegAccumulator );
        }
    }

    if ( *minValue ) *minValue = minMaxAccumulator.min;
    if ( *maxValue ) *maxValue = minMaxAccumulator.max;
    if ( *posClosestToZero ) *posClosestToZero = posNegAccumulator.pos;
    if ( *negClosestToZero ) *negClosestToZero = posNegAccumulator.neg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::createAndAssignTemplateCopyForNonMatchingUnit()
{
    // If a fracture has different unit than the associated template, create a copy of template in correct unit

    std::vector<RimFractureTemplate*> templatesToBeAdded;

    for ( RimFractureTemplate* fractureTemplate : m_fractureDefinitions() )
    {
        if ( fractureTemplate )
        {
            RimFractureTemplate* templateWithMatchingUnit = nullptr;

            std::vector<RimFracture*> referringObjects = fractureTemplate->objectsWithReferringPtrFieldsOfType<RimFracture>();
            for ( auto fracture : referringObjects )
            {
                if ( fracture && fracture->fractureUnit() != fractureTemplate->fractureTemplateUnit() )
                {
                    if ( !templateWithMatchingUnit )
                    {
                        templateWithMatchingUnit = dynamic_cast<RimFractureTemplate*>(
                            fractureTemplate->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

                        auto currentUnit = fractureTemplate->fractureTemplateUnit();
                        auto neededUnit  = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
                        if ( currentUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
                        {
                            neededUnit = RiaDefines::EclipseUnitSystem::UNITS_FIELD;
                        }
                        else if ( currentUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
                        {
                            neededUnit = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
                        }

                        templateWithMatchingUnit->convertToUnitSystem( neededUnit );

                        QString name = templateWithMatchingUnit->name();
                        name += " (created to match fracture unit)";
                        templateWithMatchingUnit->setName( name );

                        templatesToBeAdded.push_back( templateWithMatchingUnit );
                    }

                    RiaLogging::warning( "Detected fracture with different unit than fracture template. Creating copy of template "
                                         "with matching unit." );

                    CVF_ASSERT( templateWithMatchingUnit->fractureTemplateUnit() == fracture->fractureUnit() );
                    fracture->setFractureTemplateNoUpdate( templateWithMatchingUnit );
                }
            }
        }
    }

    for ( auto templateWithMatchingUnit : templatesToBeAdded )
    {
        m_fractureDefinitions.push_back( templateWithMatchingUnit );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::loadAndUpdateData()
{
    for ( RimFractureTemplate* f : m_fractureDefinitions() )
    {
        RimMeshFractureTemplate* stimPlanFracture = dynamic_cast<RimMeshFractureTemplate*>( f );
        if ( stimPlanFracture )
        {
            stimPlanFracture->loadDataAndUpdate();
        }

        RimEllipseFractureTemplate* ellipseFracture = dynamic_cast<RimEllipseFractureTemplate*>( f );
        if ( ellipseFracture )
        {
            ellipseFracture->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::initAfterRead()
{
    // Assign template id if not already assigned
    for ( auto& templ : m_fractureDefinitions )
    {
        if ( templ->id() < 0 ) templ->setId( nextFractureTemplateId() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFractureTemplateCollection::nextFractureTemplateId()
{
    int newId                     = m_nextValidFractureTemplateId;
    m_nextValidFractureTemplateId = m_nextValidFractureTemplateId + 1;

    return newId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                    std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    std::vector<Rim3dView*> views;
    proj->allVisibleViews( views );
    for ( Rim3dView* visibleView : views )
    {
        if ( dynamic_cast<RimEclipseView*>( visibleView ) )
        {
            visibleView->updateConnectedEditors();
        }
    }
}
