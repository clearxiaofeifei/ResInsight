/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigWbsParameter.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimWellLogCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

#include <optional>

class RigFemResultAddress;
class RigGeoMechWellLogExtractor;
class RigWellPath;
class RimCase;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class Rim3dView;
class RimWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogExtractionCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogExtractionCurve();
    ~RimWellLogExtractionCurve() override;

    enum TrajectoryType
    {
        WELL_PATH,
        SIMULATION_WELL
    };

    void         setWellPath( RimWellPath* wellPath );
    RimWellPath* wellPath() const;

    void setFromSimulationWellName( const QString& simWellName, int branchIndex, bool branchDetection );

    void     setCase( RimCase* rimCase );
    RimCase* rimCase() const;

    void setPropertiesFromView( Rim3dView* view );

    TrajectoryType trajectoryType() const;
    QString        wellName() const override;
    QString        wellLogChannelUiName() const override;
    QString        wellLogChannelName() const override;
    QString        wellLogChannelUnits() const override;
    QString        wellDate() const override;
    int            branchIndex() const;
    bool           branchDetection() const;

    bool    isEclipseCurve() const;
    QString caseName() const;

    int  currentTimeStep() const;
    void setCurrentTimeStep( int timeStep );

    void    setEclipseResultVariable( const QString& resVarname );
    QString eclipseResultVariable() const;
    void    setEclipseResultCategory( RiaDefines::ResultCatType catType );

    void setGeoMechResultAddress( const RigFemResultAddress& resAddr );
    void setGeoMechPart( int partId );
    int  geoMechPart() const;

    void setTrajectoryType( TrajectoryType trajectoryType );
    void setWellName( QString wellName );
    void setBranchDetection( bool branchDetection );
    void setBranchIndex( int index );

    static void findAndLoadWbsParametersFromLasFiles( const RimWellPath* wellPath, RigGeoMechWellLogExtractor* geomExtractor );

    void                  setAutoNameComponents( bool addCaseName, bool addProperty, bool addWellname, bool addTimeStep, bool addDate );
    RiaDefines::PhaseType phaseType() const override;

    static QString wellDateFromGridCaseModel( RimCase* gridCaseModel, int timeStep );

protected:
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;
    void    onCaseSettingsChanged( const caf::SignalEmitter* emitter );
    void    connectCaseSignals( RimCase* rimCase );

    virtual void performDataExtraction( bool* isUsingPseudoLength );
    void         extractData( bool*                        isUsingPseudoLength,
                              const std::optional<double>& smoothingThreshold,
                              const std::optional<double>& maxDistanceBetweenCurvePoints );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;

    static QString dataSourceGroupKeyword();

    void              setLogScaleFromSelectedResult();
    void              clampTimestep();
    void              clampBranchIndex();
    std::set<QString> sortedSimWellNames();
    void              clearGeneratedSimWellPaths();

    caf::PdmPtrField<RimCase*>                  m_case;
    caf::PdmField<caf::AppEnum<TrajectoryType>> m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>              m_wellPath;
    caf::PdmPtrField<RimWellPath*>              m_refWellPath;
    caf::PdmField<QString>                      m_simWellName;
    caf::PdmField<int>                          m_branchIndex;
    caf::PdmField<bool>                         m_branchDetection;

    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmField<int>                              m_geomPartId;

    caf::PdmField<bool> m_addCaseNameToCurveName;
    caf::PdmField<bool> m_addPropertyToCurveName;
    caf::PdmField<bool> m_addWellNameToCurveName;
    caf::PdmField<bool> m_addTimestepToCurveName;
    caf::PdmField<bool> m_addDateToCurveName;

    std::vector<const RigWellPath*> m_wellPathsWithExtractors;

private:
    struct WellLogExtractionCurveData
    {
        std::vector<double>       values              = std::vector<double>();
        std::vector<double>       measuredDepthValues = std::vector<double>();
        std::vector<double>       tvDepthValues       = std::vector<double>();
        double                    rkbDiff             = 0.0;
        RiaDefines::DepthUnitType depthUnit           = RiaDefines::DepthUnitType::UNIT_METER;
        QString                   xUnits              = RiaWellLogUnitTools<double>::noUnitString();
    };

private:
    WellLogExtractionCurveData extractEclipseData( RimEclipseCase* eclipseCase, bool* isUsingPseudoLength );
    WellLogExtractionCurveData extractGeomData( RimGeoMechCase*              geoMechCase,
                                                bool*                        isUsingPseudoLength,
                                                const std::optional<double>& smoothingThreshold,
                                                const std::optional<double>& maxDistanceBetweenCurvePoints );

    void mapPropertyValuesFromReferenceWell( std::vector<double>&       rMeasuredDepthValues,
                                             std::vector<double>&       rTvDepthValues,
                                             std::vector<double>&       rPropertyValues,
                                             const std::vector<double>& indexKValues,
                                             const std::vector<double>& refWellMeasuredDepthValues,
                                             const std::vector<double>& refWellTvDepthValues,
                                             const std::vector<double>& refWellPropertyValues,
                                             const std::vector<double>& refWellIndexKValues );
};
