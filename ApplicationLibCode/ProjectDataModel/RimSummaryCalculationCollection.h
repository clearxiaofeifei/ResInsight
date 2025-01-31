/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryCalculation.h"
#include "RimUserDefinedCalculationCollection.h"

#include "cafPdmObject.h"

class RimSummaryCase;
class RimCalculatedSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCalculationCollection : public RimUserDefinedCalculationCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculationCollection();

    void rebuildCaseMetaData() override;

    RimSummaryCalculation* createCalculation() const override;

private:
    void initAfterRead() override;
};
