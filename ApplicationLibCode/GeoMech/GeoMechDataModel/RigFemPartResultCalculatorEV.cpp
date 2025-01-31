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

#include "RigFemPartResultCalculatorEV.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorEV::RigFemPartResultCalculatorEV( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorEV::~RigFemPartResultCalculatorEV()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorEV::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorEV::calculate( int partIndex, const RigFemResultAddress& resAddr )
{
    CVF_ASSERT( resAddr.fieldName == "NE" && resAddr.componentName == "EV" );

    QString progressText = "Calculating " + QString::fromStdString( resAddr.fieldName + ": " + resAddr.componentName );

    caf::ProgressInfo stepCountProgress( static_cast<size_t>( m_resultCollection->timeStepCount() ) * 4, progressText );

    auto loadFrameLambda = [&]( const QString& component )
    {
        auto task = stepCountProgress.task( "Loading " + component, m_resultCollection->timeStepCount() );
        return m_resultCollection->findOrLoadScalarResult( partIndex, resAddr.copyWithComponent( component.toStdString() ) );
    };

    RigFemScalarResultFrames* ea11 = loadFrameLambda( "E11" );
    RigFemScalarResultFrames* ea22 = loadFrameLambda( "E22" );
    RigFemScalarResultFrames* ea33 = loadFrameLambda( "E33" );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resAddr );

    const int timeSteps = ea11->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        auto task = stepCountProgress.task( QString( "Step %1" ).arg( stepIdx ) );

        const int frameCount = ea11->frameCount( stepIdx );
        for ( int fIdx = 0; fIdx < frameCount; fIdx++ )
        {
            const std::vector<float>& ea11Data = ea11->frameData( stepIdx, fIdx );
            const std::vector<float>& ea22Data = ea22->frameData( stepIdx, fIdx );
            const std::vector<float>& ea33Data = ea33->frameData( stepIdx, fIdx );

            std::vector<float>& dstFrameData = dstDataFrames->frameData( stepIdx, fIdx );
            size_t              valCount     = ea11Data.size();
            dstFrameData.resize( valCount );

#pragma omp parallel for
            for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
            {
                dstFrameData[vIdx] = ( ea11Data[vIdx] + ea22Data[vIdx] + ea33Data[vIdx] );
            }
        }
    }
    return dstDataFrames;
}
