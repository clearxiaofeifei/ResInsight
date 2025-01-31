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

#include "RicReplaceSummaryCaseFeature.h"

#include "RiaLogging.h"
#include "RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimFileSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReplaceSummaryCaseFeature, "RicReplaceSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::updateRequredCalculatedCurves( RimSummaryCase* sourceSummaryCase )
{
    RimSummaryCalculationCollection* calcColl = RimProject::current()->calculationCollection();

    for ( RimUserDefinedCalculation* summaryCalculation : calcColl->calculations() )
    {
        bool needsUpdate = RicReplaceSummaryCaseFeature::checkIfCalculationNeedsUpdate( summaryCalculation, sourceSummaryCase );
        if ( needsUpdate )
        {
            summaryCalculation->parseExpression();
            summaryCalculation->calculate();
            summaryCalculation->updateDependentObjects();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryCaseFeature::isCommandEnabled()
{
    RimSummaryCase* rimSummaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    return rimSummaryCase != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    RimFileSummaryCase* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    if ( !summaryCase ) return;

    const QStringList fileNames =
        RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType::ECLIPSE_SUMMARY_FILE );
    if ( fileNames.isEmpty() ) return;

    QString oldSummaryHeaderFilename = summaryCase->summaryHeaderFilename();
    summaryCase->setSummaryHeaderFileName( fileNames[0] );
    summaryCase->updateAutoShortName();
    summaryCase->createSummaryReaderInterface();
    summaryCase->createRftReaderInterface();
    summaryCase->refreshMetaData();

    RiaLogging::info( QString( "Replaced summary data for %1" ).arg( oldSummaryHeaderFilename ) );

    RicReplaceSummaryCaseFeature::updateRequredCalculatedCurves( summaryCase );

    // Find and update all changed calculations
    std::set<int>                    ids;
    RimSummaryCalculationCollection* calcColl = RimProject::current()->calculationCollection();
    for ( RimUserDefinedCalculation* summaryCalculation : calcColl->calculations() )
    {
        bool needsUpdate = checkIfCalculationNeedsUpdate( summaryCalculation, summaryCase );
        if ( needsUpdate )
        {
            ids.insert( summaryCalculation->id() );
        }
    }

    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
    for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
        {
            summaryPlot->loadDataAndUpdate();
        }
        multiPlot->updatePlotTitles();
    }

    RimSummaryCrossPlotCollection* summaryCrossPlotColl = RiaSummaryTools::summaryCrossPlotCollection();
    for ( RimSummaryPlot* summaryPlot : summaryCrossPlotColl->plots() )
    {
        summaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Replace" );
    actionToSetup->setIcon( QIcon( ":/ReplaceCase16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryCaseFeature::checkIfCalculationNeedsUpdate( const RimUserDefinedCalculation* summaryCalculation,
                                                                  const RimSummaryCase*            summaryCase )
{
    std::vector<RimUserDefinedCalculationVariable*> variables = summaryCalculation->allVariables();
    for ( RimUserDefinedCalculationVariable* variable : variables )
    {
        RimSummaryCalculationVariable* summaryVariable = dynamic_cast<RimSummaryCalculationVariable*>( variable );
        if ( summaryVariable->summaryCase() == summaryCase )
        {
            return true;
        }
    }

    return false;
}
