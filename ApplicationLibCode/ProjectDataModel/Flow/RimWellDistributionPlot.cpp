/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDistributionPlot.h"

#include "RimEclipseCaseTools.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimTools.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTools.h"

#include "RiuQwtPlotWidget.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QGridLayout>
#include <QTextBrowser>
#include <QWidget>

#include <array>

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellDistributionPlot, "WellDistributionPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::RimWellDistributionPlot( RiaDefines::PhaseType phase )
{
    // cvf::Trace::show("RimWellDistributionPlot::RimWellDistributionPlot()");

    CAF_PDM_InitObject( "Cumulative Phase Distribution Plot" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case" );
    CAF_PDM_InitField( &m_timeStepIndex, "TimeStepIndex", -1, "Time Step" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well" );
    CAF_PDM_InitField( &m_phase, "Phase", caf::AppEnum<RiaDefines::PhaseType>( phase ), "Phase" );
    CAF_PDM_InitField( &m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions" );
    CAF_PDM_InitField( &m_smallContributionsRelativeThreshold, "SmallContributionsRelativeThreshold", 0.005, "Relative Threshold [0, 1]" );

    CAF_PDM_InitField( &m_maximumTof, "MaximumTOF", 20.0, "Maximum Time of Flight [0, 200]" );

    m_showWindow      = false;
    m_showPlotLegends = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::~RimWellDistributionPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::setDataSourceParameters( RimEclipseResultCase* eclipseResultCase, int timeStepIndex, QString targetWellName )
{
    m_case          = eclipseResultCase;
    m_timeStepIndex = timeStepIndex;
    m_wellName      = targetWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::setPlotOptions( bool groupSmallContributions, double smallContributionsRelativeThreshold, double maximumTof )
{
    m_groupSmallContributions             = groupSmallContributions;
    m_smallContributionsRelativeThreshold = smallContributionsRelativeThreshold;
    m_maximumTof                          = maximumTof;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimWellDistributionPlot::phase() const
{
    return m_phase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimWellDistributionPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::setAutoScaleXEnabled( bool /*enabled*/ )
{
    // cvf::Trace::show("RimWellDistributionPlot::setAutoScaleXEnabled()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::setAutoScaleYEnabled( bool /*enabled*/ )
{
    // cvf::Trace::show("RimWellDistributionPlot::setAutoScaleYEnabled()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::updateAxes()
{
    // cvf::Trace::show("RimWellDistributionPlot::updateAxes()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::updateLegend()
{
    // cvf::Trace::show("RimWellDistributionPlot::updateLegend()");

    if ( !m_plotWidget )
    {
        return;
    }

    bool doShowLegend = true;
    if ( doShowLegend )
    {
        QwtLegend* legend = new QwtLegend( m_plotWidget );
        m_plotWidget->qwtPlot()->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_plotWidget->qwtPlot()->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::updateZoomInParentPlot()
{
    // cvf::Trace::show("RimWellDistributionPlot::updateZoomInParentPlot()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::updateZoomFromParentPlot()
{
    // cvf::Trace::show("RimWellDistributionPlot::updateZoomFromParentPlot()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellDistributionPlot::asciiDataForPlotExport() const
{
    // cvf::Trace::show("RimWellDistributionPlot::asciiDataForPlotExport()");
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::reattachAllCurves()
{
    // cvf::Trace::show("RimWellDistributionPlot::reattachAllCurves()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::detachAllCurves()
{
    // cvf::Trace::show("RimWellDistributionPlot::detachAllCurves()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimWellDistributionPlot::findPdmObjectFromPlotCurve( const RiuPlotCurve* /*curve*/ ) const
{
    // cvf::Trace::show("RimWellDistributionPlot::findPdmObjectFromQwtCurve()");
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::onAxisSelected( RiuPlotAxis /*axis*/, bool /*toggle*/ )
{
    // cvf::Trace::show("RimWellDistributionPlot::onAxisSelected()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellDistributionPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlot::viewWidget()
{
    // cvf::Trace::show("RimWellDistributionPlot::viewWidget()");
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellDistributionPlot::snapshotWindowContent()
{
    // cvf::Trace::show("RimWellDistributionPlot::snapshotWindowContent()");

    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::zoomAll()
{
    // cvf::Trace::show("RimWellDistributionPlot::zoomAll()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimWellDistributionPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    // cvf::Trace::show("RimWellDistributionPlot::createViewWidget()");

    // It seems we risk being called multiple times
    if ( m_plotWidget )
    {
        return m_plotWidget;
    }

    m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

    m_plotWidget->qwtPlot()->setAutoReplot( false );

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::deleteViewWidget()
{
    // cvf::Trace::show("RimWellDistributionPlot::deleteViewWidget()");

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::onLoadDataAndUpdate()
{
    // cvf::Trace::show("RimWellDistributionPlot::onLoadDataAndUpdate()");
    // cvf::DebugTimer tim("RimWellDistributionPlot::onLoadDataAndUpdate()");

    updateParentLayout();

    if ( !m_plotWidget )
    {
        return;
    }

    m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotCurve );

    updateLegend();

    if ( m_case && m_case->ensureReservoirCaseIsOpen() )
    {
        // tim.reportLapTimeMS("about to start calc");
        RigTofWellDistributionCalculator calc( m_case, m_wellName, m_timeStepIndex, m_phase() );
        // tim.reportLapTimeMS("calc");

        if ( m_groupSmallContributions )
        {
            calc.groupSmallContributions( m_smallContributionsRelativeThreshold );
            // tim.reportLapTimeMS("group");
        }

        const RimFlowDiagSolution* flowDiagSolution = m_case->defaultFlowDiagSolution();

        // cvf::Trace::show("Populating plot for phase '%s'", m_phase == RiaDefines::OIL_PHASE ? "oil" : (m_phase ==
        // RiaDefines::GAS_PHASE ? "gas" : "water"));
        populatePlotWidgetWithCurveData( calc, *flowDiagSolution, m_plotWidget, m_maximumTof );
    }

    QString phaseString = "N/A";
    if ( m_phase == RiaDefines::PhaseType::OIL_PHASE )
        phaseString = "Oil";
    else if ( m_phase == RiaDefines::PhaseType::GAS_PHASE )
        phaseString = "Gas";
    else if ( m_phase == RiaDefines::PhaseType::WATER_PHASE )
        phaseString = "Water";

    const QString timeStepName = m_case ? m_case->timeStepName( m_timeStepIndex ) : "N/A";

    const QString plotTitleStr = QString( "%1 Distribution: %2, %3" ).arg( phaseString ).arg( m_wellName ).arg( timeStepName );
    m_plotWidget->setPlotTitleRenderingFlags( Qt::AlignHCenter | Qt::TextWordWrap );
    m_plotWidget->setPlotTitle( plotTitleStr );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "TOF [years]" );
    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), "Reservoir Volume [m3]" );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::populatePlotWidgetWithCurveData( const RigTofWellDistributionCalculator& calculator,
                                                               const RimFlowDiagSolution&              flowDiagSolution,
                                                               RiuQwtPlotWidget*                       plotWidget,
                                                               double                                  maximumTof )
{
    // cvf::Trace::show("RimWellDistributionPlot::populatePlotWidgetWithCurves()");

    // Currently select this value so that the grid appears on top of the curves
    const double baseCurveZValue = 9.5;

    plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( RiuPlotAxis::defaultBottom(), 0, 1 );
    plotWidget->setAxisScale( RiuPlotAxis::defaultLeft(), 0, 1 );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultBottom(), true );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultLeft(), true );

    const std::vector<double>& tofValuesDays = calculator.sortedUniqueTofValues();
    if ( tofValuesDays.size() == 0 )
    {
        // cvf::Trace::show("No TOF values!");
        return;
    }

    std::vector<double> tofValuesYears;
    for ( double tofDays : tofValuesDays )
    {
        const double tofYears = tofDays / 365.2425;

        if ( tofYears > maximumTof ) continue;

        tofValuesYears.push_back( tofYears );
    }

    // cvf::Trace::show("numTofValues: %d  (min, max: %f, %f)", static_cast<int>(tofValuesYears.size()),
    // tofValuesYears.front(), tofValuesYears.back());

    const size_t numWells = calculator.contributingWellCount();
    // cvf::Trace::show("numContribWells: %d", static_cast<int>(numWells));

    std::vector<double> yVals( tofValuesYears.size(), 0 );

    for ( size_t i = 0; i < numWells; i++ )
    {
        const QString&             wellName = calculator.contributingWellName( i );
        const std::vector<double>& volArr   = calculator.accumulatedVolumeForContributingWell( i );

        cvf::Color3f cvfClr = flowDiagSolution.tracerColor( wellName );
        QColor       qtClr  = RiaColorTools::toQColor( cvfClr );

        for ( size_t j = 0; j < yVals.size(); j++ )
        {
            yVals[j] += volArr[j];
        }

        // cvf::Trace::show("wellName  min, max: %15s   %12.3f, %12.3f     maxAggrY: %12.3f",
        // wellName.toStdString().c_str(), volArr.front(), volArr.back(), yVals.back());

        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setTitle( wellName );
        curve->setBrush( qtClr );
        curve->setZ( baseCurveZValue - i * 0.0001 );
        curve->setSamples( tofValuesYears.data(), yVals.data(), static_cast<int>( tofValuesYears.size() ) );
        curve->attach( plotWidget->qwtPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    uiOrdering.add( &m_timeStepIndex );
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_phase );
    uiOrdering.add( &m_groupSmallContributions );
    uiOrdering.add( &m_smallContributionsRelativeThreshold );

    m_smallContributionsRelativeThreshold.uiCapability()->setUiReadOnly( m_groupSmallContributions == false );

    RimPlot::defineUiOrdering( uiConfigName, uiOrdering );
    // uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellDistributionPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_case )
    {
        auto resultCases = RimEclipseCaseTools::eclipseResultCases();
        for ( RimEclipseResultCase* c : resultCases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, true, c->uiIconProvider() ) );
        }
    }

    else if ( fieldNeedingOptions == &m_timeStepIndex )
    {
        RimTools::timeStepsForCase( m_case, &options );

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", -1 ) );
        }
    }

    else if ( fieldNeedingOptions == &m_wellName )
    {
        if ( m_case && m_case->eclipseCaseData() )
        {
            caf::IconProvider       simWellIcon( ":/Well.svg" );
            const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
            for ( const QString& name : sortedWellNameSet )
            {
                options.push_back( caf::PdmOptionItemInfo( name, name, true, simWellIcon ) );
            }
        }

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", QVariant() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        fixupDependentFieldsAfterCaseChange();
    }

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::fixupDependentFieldsAfterCaseChange()
{
    int     newTimeStepIndex = -1;
    QString newWellName;

    if ( m_case )
    {
        const int timeStepCount = m_case->timeStepStrings().size();
        if ( timeStepCount > 0 )
        {
            newTimeStepIndex = timeStepCount - 1;
        }

        const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
        if ( sortedWellNameSet.size() > 0 )
        {
            newWellName = *sortedWellNameSet.begin();
        }
    }

    m_timeStepIndex = newTimeStepIndex;
    m_wellName      = newWellName;
}
