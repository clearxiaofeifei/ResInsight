/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimTimeStepFilter.h"

#include "RiaQDateTimeTools.h"

#include "RifReaderEclipseOutput.h"

#include "RigCaseCellResultsData.h"

#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QDateTime>

#include <algorithm>

namespace caf
{
template <>
void caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>::setUp()
{
    addItem( RimTimeStepFilter::TS_ALL, "TS_ALL", "All" );
    addItem( RimTimeStepFilter::TS_INTERVAL_DAYS, "TS_INTERVAL_DAYS", "Skip by Days" );
    addItem( RimTimeStepFilter::TS_INTERVAL_WEEKS, "TS_INTERVAL_WEEKS", "Skip by Weeks" );
    addItem( RimTimeStepFilter::TS_INTERVAL_MONTHS, "TS_INTERVAL_MONTHS", "Skip by Months" );
    addItem( RimTimeStepFilter::TS_INTERVAL_QUARTERS, "TS_INTERVAL_QUARTERS", "Skip by Quarters" );
    addItem( RimTimeStepFilter::TS_INTERVAL_YEARS, "TS_INTERVAL_YEARS", "Skip by Years" );

    setDefault( RimTimeStepFilter::TS_ALL );
}

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimTimeStepFilter, "RimTimeStepFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeStepFilter::RimTimeStepFilter()
{
    CAF_PDM_InitObject( "Time Step Filter" );

    caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum> filterType = TS_ALL;
    CAF_PDM_InitField( &m_filterType, "FilterType", filterType, "Filter Type" );

    CAF_PDM_InitField( &m_firstTimeStep, "FirstTimeStep", 0, "First Time Step" );
    CAF_PDM_InitField( &m_lastTimeStep, "LastTimeStep", 0, "Last Time Step" );

    CAF_PDM_InitField( &m_interval, "Interval", 1, "Interval" );
    m_interval.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_timeStepNamesFromFile, "TimeStepsFromFile", std::vector<QString>(), "TimeSteps From File" );
    m_timeStepNamesFromFile.xmlCapability()->disableIO();
    CAF_PDM_InitField( &m_dateFormat, "DateFormat", QString( "yyyy-MM-dd" ), "Date Format" );

    CAF_PDM_InitFieldNoDefault( &m_filteredTimeSteps, "TimeStepIndicesToImport", "Select From Time Steps" );
    m_filteredTimeSteps.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_filteredTimeStepsUi, "TimeStepIndicesUi", "Select From TimeSteps" );
    m_filteredTimeStepsUi.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_filteredTimeStepsUi.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_readOnlyLastFrame, "OnlyLastFrame", false, "Load Only Last Frame Of Each Time Step" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_readOnlyLastFrame );

    CAF_PDM_InitFieldNoDefault( &m_applyReloadOfCase, "ApplyReloadOfCase", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_applyReloadOfCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.1" ) )
    {
        m_readOnlyLastFrame = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::clearFilteredTimeSteps()
{
    m_filteredTimeSteps = std::vector<int>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setTimeStepsFromFile( const std::vector<QDateTime>& timeSteps )
{
    m_dateFormat = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

    std::vector<QString> timeStepStrings;
    for ( const QDateTime& date : timeSteps )
    {
        QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale( date, m_dateFormat );

        timeStepStrings.push_back( dateString );
    }

    m_timeStepNamesFromFile = timeStepStrings;
    m_lastTimeStep          = static_cast<int>( timeSteps.size() ) - 1;

    if ( m_filteredTimeSteps().empty() )
    {
        m_filteredTimeSteps = filteredTimeStepIndicesFromUi();
    }
    m_filteredTimeStepsUi = m_filteredTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setTimeStepsFromFile( const std::vector<std::pair<QString, QDateTime>>& timeSteps )
{
    std::vector<QDateTime> validDates;
    for ( auto stringDatePair : timeSteps )
    {
        if ( stringDatePair.second.isValid() )
        {
            validDates.push_back( stringDatePair.second );
        }
    }
    m_dateFormat = RiaQDateTimeTools::createTimeFormatStringFromDates( validDates );

    std::vector<QString> timeStepStrings;
    for ( auto stringDatePair : timeSteps )
    {
        QString stepString = stringDatePair.first;
        if ( stringDatePair.second.isValid() )
        {
            stepString = RiaQDateTimeTools::toStringUsingApplicationLocale( stringDatePair.second, m_dateFormat );
        }
        timeStepStrings.push_back( stepString );
    }

    m_timeStepNamesFromFile = timeStepStrings;
    m_lastTimeStep          = static_cast<int>( timeSteps.size() ) - 1;

    if ( m_filteredTimeSteps().empty() )
    {
        m_filteredTimeSteps = filteredTimeStepIndicesFromUi();
    }
    m_filteredTimeStepsUi = m_filteredTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimTimeStepFilter::filteredTimeSteps() const
{
    std::vector<size_t> indices;

    // Convert vector from int to size_t
    for ( int index : m_filteredTimeSteps() )
    {
        indices.push_back( static_cast<size_t>( index ) );
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTimeStepFilter::updateFilteredTimeStepsFromUi()
{
    std::vector<int> timeSteps = m_filteredTimeStepsUi;
    std::sort( timeSteps.begin(), timeSteps.end() );
    if ( m_filteredTimeSteps() == timeSteps )
    {
        return false;
    }
    m_filteredTimeSteps = timeSteps;
    return true;
}

QDateTime RimTimeStepFilter::incrementDateTime( const QDateTime& dateTime, TimeStepFilterTypeEnum filterType, int interval )
{
    switch ( filterType )
    {
        case TS_INTERVAL_DAYS:
            return dateTime.addDays( interval );
        case TS_INTERVAL_WEEKS:
            return dateTime.addDays( 7 * interval );
        case TS_INTERVAL_MONTHS:
            return dateTime.addMonths( interval );
        case TS_INTERVAL_QUARTERS:
            return dateTime.addMonths( interval * 3 );
        case TS_INTERVAL_YEARS:
            return dateTime.addYears( interval );
        default:
            break;
    }
    return dateTime;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimTimeStepFilter::filteredTimeStepIndices( const std::vector<QDateTime>& timeSteps,
                                                             int                           firstTimeStep,
                                                             int                           lastTimeStep,
                                                             TimeStepFilterTypeEnum        filterType,
                                                             int                           interval )
{
    std::vector<int> indices;

    if ( filterType == TS_ALL )
    {
        for ( int i = firstTimeStep; i <= lastTimeStep; i++ )
        {
            indices.push_back( i );
        }
    }
    else
    {
        QDateTime nextDateTime;
        for ( int i = firstTimeStep; i <= lastTimeStep; i++ )
        {
            if ( !timeSteps[i].isValid() )
            {
                indices.push_back( i );
            }
            else
            {
                if ( nextDateTime.isValid() )
                {
                    if ( timeSteps[i] >= nextDateTime )
                    {
                        do
                        {
                            nextDateTime = incrementDateTime( nextDateTime, filterType, interval );
                        } while ( nextDateTime <= timeSteps[i] );

                        indices.push_back( i );
                    }
                }
                else
                {
                    nextDateTime = incrementDateTime( timeSteps[i], filterType, interval );
                    indices.push_back( i );
                }
            }
        }
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimEclipseResultCase* rimEclipseResultCase = parentEclipseResultCase();
    RimGeoMechCase*       rimGeoMechCase       = parentGeoMechCase();
    if ( changedField == &m_applyReloadOfCase )
    {
        updateFilteredTimeStepsFromUi();

        if ( rimEclipseResultCase )
        {
            RimReloadCaseTools::reloadAllEclipseGridData( rimEclipseResultCase );
        }
        else if ( rimGeoMechCase )
        {
            rimGeoMechCase->reloadDataAndUpdate();
        }

        return;
    }

    if ( changedField == &m_filterType || changedField == &m_firstTimeStep || changedField == &m_lastTimeStep || changedField == &m_interval )
    {
        m_filteredTimeStepsUi = filteredTimeStepIndicesFromUi();
    }

    if ( rimEclipseResultCase )
    {
        rimEclipseResultCase->updateConnectedEditors();
    }
    else if ( rimGeoMechCase )
    {
        rimGeoMechCase->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimTimeStepFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> optionItems;

    if ( fieldNeedingOptions == &m_firstTimeStep || fieldNeedingOptions == &m_lastTimeStep )
    {
        for ( size_t i = 0; i < m_timeStepNamesFromFile().size(); i++ )
        {
            optionItems.push_back( caf::PdmOptionItemInfo( m_timeStepNamesFromFile()[i], static_cast<int>( i ) ) );
        }
    }

    if ( fieldNeedingOptions == &m_filteredTimeStepsUi )
    {
        std::vector<int> filteredTimeSteps = filteredTimeStepIndicesFromUi();
        for ( auto filteredIndex : filteredTimeSteps )
        {
            if ( filteredIndex < static_cast<int>( m_timeStepNamesFromFile().size() ) )
            {
                optionItems.push_back( caf::PdmOptionItemInfo( m_timeStepNamesFromFile()[filteredIndex], static_cast<int>( filteredIndex ) ) );
            }
        }
    }

    return optionItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_applyReloadOfCase )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Reload Case";
        }
    }
    else if ( field == &m_interval )
    {
        caf::PdmUiLineEditorAttribute* attrib = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->avoidSendingEnterEventToParentWidget = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QDateTime>> RimTimeStepFilter::allTimeSteps() const
{
    std::vector<std::pair<QString, QDateTime>> timeSteps;
    for ( const QString& dateString : m_timeStepNamesFromFile() )
    {
        timeSteps.push_back( std::make_pair( dateString, QDateTime::fromString( dateString, m_dateFormat ) ) );
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimTimeStepFilter::filteredTimeStepIndicesFromUi() const
{
    std::vector<std::pair<QString, QDateTime>> timeStepStringsAndDate = allTimeSteps();
    std::vector<QDateTime>                     timeSteps;
    for ( auto stringDatePair : timeStepStringsAndDate )
    {
        timeSteps.push_back( stringDatePair.second );
    }
    return filteredTimeStepIndices( timeSteps, m_firstTimeStep(), m_lastTimeStep(), m_filterType(), m_interval );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::updateFieldVisibility()
{
    if ( m_filterType == TS_ALL )
    {
        m_interval.uiCapability()->setUiHidden( true );
    }
    else
    {
        m_interval.uiCapability()->setUiHidden( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimTimeStepFilter::parentEclipseResultCase() const
{
    return firstAncestorOrThisOfType<RimEclipseResultCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimTimeStepFilter::parentGeoMechCase() const
{
    return firstAncestorOrThisOfType<RimGeoMechCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filterType );
    uiOrdering.add( &m_firstTimeStep );
    uiOrdering.add( &m_lastTimeStep );
    uiOrdering.add( &m_interval );
    uiOrdering.add( &m_filteredTimeStepsUi );
    size_t  numberOfFilterOptions = filteredTimeStepIndicesFromUi().size();
    QString displayUiName         = QString( "Select From %1 Time Steps:" ).arg( numberOfFilterOptions );
    m_filteredTimeStepsUi.uiCapability()->setUiName( displayUiName );

    bool caseLoaded = false;

    RimEclipseResultCase* eclipseCase = parentEclipseResultCase();
    RimGeoMechCase*       geoMechCase = parentGeoMechCase();

    if ( eclipseCase )
    {
        caseLoaded = eclipseCase->eclipseCaseData() != nullptr;
    }
    else if ( geoMechCase )
    {
        caseLoaded = geoMechCase->geoMechData() != nullptr;

        uiOrdering.add( &m_readOnlyLastFrame );
    }

    if ( caseLoaded )
    {
        uiOrdering.add( &m_applyReloadOfCase );
    }

    updateFieldVisibility();

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setReadOnlyLastFrame( bool onlyLast )
{
    m_readOnlyLastFrame = onlyLast;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTimeStepFilter::readOnlyLastFrame() const
{
    return m_readOnlyLastFrame;
}
