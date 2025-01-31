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
#pragma once

#include "RiaDefines.h"
#include "RiaResultNames.h"

#include <QString>

class RigEclipseResultAddress
{
public:
    RigEclipseResultAddress()
        : m_resultCatType( RiaDefines::ResultCatType::UNDEFINED )
        , m_resultDataType( RiaDefines::ResultDataType::UNKNOWN )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_differenceCaseId( NO_CASE_DIFF )
        , m_divideByCellFaceArea( false )
    {
    }

    explicit RigEclipseResultAddress( const QString& resultName )
        : m_resultCatType( RiaDefines::ResultCatType::UNDEFINED )
        , m_resultDataType( RiaDefines::ResultDataType::UNKNOWN )
        , m_resultName( resultName )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_differenceCaseId( NO_CASE_DIFF )
        , m_divideByCellFaceArea( false )
    {
    }

    explicit RigEclipseResultAddress( RiaDefines::ResultCatType type,
                                      const QString&            resultName,
                                      int                       timeLapseBaseTimeStep = NO_TIME_LAPSE,
                                      int                       differenceCaseId      = NO_CASE_DIFF,
                                      bool                      divideByCellFaceArea  = false )
        : m_resultCatType( type )
        , m_resultDataType( RiaDefines::ResultDataType::UNKNOWN )
        , m_resultName( resultName )
        , m_timeLapseBaseFrameIdx( timeLapseBaseTimeStep )
        , m_differenceCaseId( differenceCaseId )
        , m_divideByCellFaceArea( divideByCellFaceArea )
    {
    }

    explicit RigEclipseResultAddress( RiaDefines::ResultCatType type, RiaDefines::ResultDataType dataType, const QString& resultName )
        : m_resultCatType( type )
        , m_resultDataType( dataType )
        , m_resultName( resultName )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_differenceCaseId( NO_CASE_DIFF )
        , m_divideByCellFaceArea( false )
    {
    }

    bool isValid() const
    {
        if ( m_resultName.isEmpty() || m_resultName == RiaResultNames::undefinedResultName() )
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    void                       setDataType( RiaDefines::ResultDataType dataType ) { m_resultDataType = dataType; }
    RiaDefines::ResultDataType dataType() const { return m_resultDataType; }

    // Delta Time Step
    bool                 isDeltaTimeStepActive() const { return m_timeLapseBaseFrameIdx > NO_TIME_LAPSE; }
    void                 setDeltaTimeStepIndex( int timeStepIndex ) { m_timeLapseBaseFrameIdx = timeStepIndex; }
    int                  deltaTimeStepIndex() const { return m_timeLapseBaseFrameIdx; }
    bool                 representsAllTimeLapses() const { return m_timeLapseBaseFrameIdx == ALL_TIME_LAPSES; }
    static constexpr int allTimeLapsesValue() { return ALL_TIME_LAPSES; }
    static constexpr int noTimeLapseValue() { return NO_TIME_LAPSE; }

    // Delta Grid Case
    bool                 isDeltaCaseActive() const { return m_differenceCaseId > NO_CASE_DIFF; }
    void                 setDeltaCaseId( int caseId ) { m_differenceCaseId = caseId; }
    int                  deltaCaseId() const { return m_differenceCaseId; }
    static constexpr int noCaseDiffValue() { return NO_CASE_DIFF; }

    // Divide by Cell Face Area
    void enableDivideByCellFaceArea( bool enable ) { m_divideByCellFaceArea = enable; }

    bool isDivideByCellFaceAreaActive() const { return m_divideByCellFaceArea; }

    bool operator<( const RigEclipseResultAddress& other ) const
    {
        if ( m_divideByCellFaceArea != other.m_divideByCellFaceArea )
        {
            return ( m_divideByCellFaceArea < other.m_divideByCellFaceArea );
        }

        if ( m_differenceCaseId != other.m_differenceCaseId )
        {
            return ( m_differenceCaseId < other.m_differenceCaseId );
        }

        if ( m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx )
        {
            return ( m_timeLapseBaseFrameIdx < other.m_timeLapseBaseFrameIdx );
        }

        if ( m_resultCatType != other.m_resultCatType )
        {
            return ( m_resultCatType < other.m_resultCatType );
        }

        return ( m_resultName < other.m_resultName );
    }

    bool operator==( const RigEclipseResultAddress& other ) const
    {
        if ( m_resultCatType != other.m_resultCatType || m_resultName != other.m_resultName ||
             m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx || m_differenceCaseId != other.m_differenceCaseId ||
             m_divideByCellFaceArea != other.m_divideByCellFaceArea )
        {
            return false;
        }

        return true;
    }

    const QString& resultName() const { return m_resultName; }
    void           setResultName( QString name ) { m_resultName = name; }

    RiaDefines::ResultCatType resultCatType() const { return m_resultCatType; }
    void                      setResultCatType( RiaDefines::ResultCatType catType ) { m_resultCatType = catType; }

private:
    RiaDefines::ResultCatType  m_resultCatType;
    RiaDefines::ResultDataType m_resultDataType;
    QString                    m_resultName;
    int                        m_timeLapseBaseFrameIdx;
    int                        m_differenceCaseId;
    bool                       m_divideByCellFaceArea;

    static const int ALL_TIME_LAPSES = -2;
    static const int NO_TIME_LAPSE   = -1;
    static const int NO_CASE_DIFF    = -1;
};
