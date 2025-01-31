/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigHexIntersectionTools.h"
#include "RigWellLogExtractionTools.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <gsl/gsl>

#include <map>
#include <vector>

//==================================================================================================
///
//==================================================================================================
struct WellPathCellIntersectionInfo
{
    size_t     globCellIndex;
    cvf::Vec3d startPoint;
    cvf::Vec3d endPoint;
    double     startMD;
    double     endMD;
    cvf::Vec3d intersectionLengthsInCellCS;

    cvf::StructGridInterface::FaceType intersectedCellFaceIn;
    cvf::StructGridInterface::FaceType intersectedCellFaceOut;

    double startTVD() const { return -startPoint.z(); }
    double endTVD() const { return -endPoint.z(); }
};

class RigWellPath;

//==================================================================================================
///
//==================================================================================================
class RigWellLogExtractor : public cvf::Object
{
public:
    RigWellLogExtractor( gsl::not_null<const RigWellPath*> wellpath, const std::string& wellCaseErrorMsgName );
    ~RigWellLogExtractor() override;

    const std::vector<double>&                             cellIntersectionMDs() const;
    const std::vector<double>&                             cellIntersectionTVDs() const;
    const std::vector<size_t>&                             intersectedCellsGlobIdx() const;
    const std::vector<cvf::Vec3d>&                         intersections() const;
    const std::vector<cvf::StructGridInterface::FaceType>& intersectedCellFaces() const;

    const RigWellPath* wellPathGeometry() const;

    std::vector<WellPathCellIntersectionInfo> cellIntersectionInfosAlongWellPath() const;

    void resampleIntersections( double maxDistanceBetweenIntersections );

protected:
    static void insertIntersectionsInMap( const std::vector<HexIntersectionInfo>&                   intersections,
                                          cvf::Vec3d                                                p1,
                                          double                                                    md1,
                                          cvf::Vec3d                                                p2,
                                          double                                                    md2,
                                          double                                                    tolerance,
                                          std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo>* uniqueIntersections );

    void populateReturnArrays( std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo>& uniqueIntersections );
    void appendIntersectionToArrays( double measuredDepth, const HexIntersectionInfo& intersection, gsl::not_null<QStringList*> errorMessages );

    virtual cvf::Vec3d calculateLengthInCell( size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint ) const = 0;

protected:
    cvf::cref<RigWellPath> m_wellPathGeometry;
    std::string            m_wellCaseErrorMsgName;

private:
    std::vector<cvf::Vec3d>                         m_intersections;
    std::vector<size_t>                             m_intersectedCellsGlobIdx;
    std::vector<cvf::StructGridInterface::FaceType> m_intersectedCellFaces;
    std::vector<double>                             m_intersectionMeasuredDepths;
    std::vector<double>                             m_intersectionTVDs;
};
