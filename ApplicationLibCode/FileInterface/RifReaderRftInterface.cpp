/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RifReaderRftInterface.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderRftInterface::eclipseRftAddresses( const QString& wellName, const QDateTime& timeStep )
{
    std::set<RifEclipseRftAddress> matchingAddresses;
    std::set<RifEclipseRftAddress> allAddresses = this->eclipseRftAddresses();
    for ( const RifEclipseRftAddress& address : allAddresses )
    {
        if ( address.wellName() == wellName && address.timeStep() == timeStep )
        {
            matchingAddresses.insert( address );
        }
    }
    return matchingAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRftInterface::cellIndices( const RifEclipseRftAddress& rftAddress, std::vector<caf::VecIjk>* indices )
{
}
