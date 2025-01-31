/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#pragma once

#include "cafCmdFeature.h"
#include "cafPdmPointer.h"

#include <vector>

class RimSummaryCurve;
class RimAsciiDataCurve;
class RicPasteAsciiDataToSummaryPlotFeatureUi;
class RimSummaryPlot;

//==================================================================================================
///
//==================================================================================================
class RicPasteAsciiDataToSummaryPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    enum CurveType
    {
        CURVE_GAS,
        CURVE_OIL,
        CURVE_WAT,
        CURVE_UNKNOWN,
    };

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static QString getPastedData();
    static bool    hasPastedText();

    static std::vector<RimAsciiDataCurve*> parseCurves( QString& data, const RicPasteAsciiDataToSummaryPlotFeatureUi& settings );

    static CurveType guessCurveType( const QString& curveName );
};
