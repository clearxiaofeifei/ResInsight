/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RifEclipseTextFileReader.h"
#include "RifEclipseKeywordContent.h"

// Utility class for fast conversion from string to float
#include "fast_float/include/fast_float/fast_float.h"

// Utility class memory mapping of files
#include "mio/mio.hpp"

#include "RiaPreferencesSystem.h"
#include "RiaStdStringTools.h"

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseKeywordContent> RifEclipseTextFileReader::readKeywordAndValues( const std::string& filename )
{
    if ( RiaPreferencesSystem::current()->eclipseTextFileReaderMode() == RiaPreferencesSystem::EclipseTextFileReaderMode::MEMORY_MAPPED_FILE )
    {
        return readKeywordAndValuesMemoryMappedFile( filename );
    }

    return readKeywordAndValuesFile( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseKeywordContent> RifEclipseTextFileReader::readKeywordAndValuesFile( const std::string& filename )
{
    std::string stringData;

    std::ifstream inFile;
    inFile.open( filename );

    std::stringstream strStream;
    strStream << inFile.rdbuf();

    stringData = strStream.str();

    return parseStringData( stringData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseKeywordContent> RifEclipseTextFileReader::readKeywordAndValuesMemoryMappedFile( const std::string& filename )
{
    if ( !std::filesystem::exists( filename ) ) return {};

    mio::mmap_source mmap( filename );

    std::error_code  error;
    mio::mmap_sink   rw_mmap    = mio::make_mmap_sink( filename, 0, mio::map_entire_file, error );
    std::string_view stringData = rw_mmap.data();

    return parseStringData( stringData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::vector<float>>
    RifEclipseTextFileReader::readKeywordAndValues( const std::string_view& stringData, const size_t offset, size_t& bytesRead )
{
    std::vector<float> values;

    auto isKeywordWithoutData = []( const std::string& keyword )
    {
        std::vector<std::string> keywords = { "ECHO", "NOECHO" };
        return std::find( keywords.begin(), keywords.end(), keyword ) != keywords.end();
    };

    const auto commentChar   = '-';
    const auto commentString = "--";

    std::string      keywordName;
    std::string_view line;
    bool             isEndTokenKeywordRead = false;

    bytesRead   = 0;
    float value = 0.0f;

    bool isFaultKeyword = false;

    while ( !isEndTokenKeywordRead && ( offset + bytesRead < stringData.size() ) )
    {
        size_t bytesReadLine = 0;
        line                 = readLine( stringData, offset + bytesRead, bytesReadLine );
        bytesRead += bytesReadLine;

        if ( isFaultKeyword )
        {
            // Read data until the FAULTS section is closed with a single / on one line
            ltrim( line );
            if ( !line.empty() && line[0] == '/' )
            {
                return std::make_pair( keywordName, values );
            }
            continue;
        }

        // Check for '--', used to define start of comments
        if ( line.size() > 2 && line[0] == commentChar && line[1] == commentChar ) continue;

        if ( keywordName.empty() )
        {
            auto found = line.find_first_of( commentString );
            if ( found != std::string::npos )
            {
                line = line.substr( 0, found );
            }

            trim( line );
            if ( !line.empty() )
            {
                keywordName = line;
                if ( keywordName == "FAULTS" ) isFaultKeyword = true;
                if ( isKeywordWithoutData( keywordName ) ) isEndTokenKeywordRead = true;
            }

            continue;
        }

        // End of keyword is defined by '/', parse values of line until '/' is found
        auto positionOfEndToken = line.find_first_of( '/' );
        if ( positionOfEndToken != std::string::npos )
        {
            line = line.substr( 0, positionOfEndToken );
        }

        if ( !isEndTokenKeywordRead )
        {
            size_t start = 0;
            size_t end   = 0;

            // Index in token for the '*' character used to define a multiplier for the float value
            //
            size_t multiplierIndex = 0;

            while ( ( start = line.find_first_not_of( RifEclipseTextFileReader::m_whiteSpace, end ) ) != std::string::npos )
            {
                end = line.find_first_of( RifEclipseTextFileReader::m_whiteSpace, start );

                int multiplier = 1;

                multiplierIndex = line.find_first_of( '*', start );
                if ( multiplierIndex < end )
                {
                    int multiplierCandidate = 1;

                    if ( RiaStdStringTools::toInt( line.substr( start, multiplierIndex - start ), multiplierCandidate ) )
                    {
                        multiplier = multiplierCandidate;
                    }

                    start = multiplierIndex + 1;
                }

                auto resultObject = fast_float::from_chars( line.data() + start, line.data() + end, value );
                if ( resultObject.ec == std::errc() )
                {
                    for ( size_t i = 0; i < static_cast<size_t>( multiplier ); i++ )
                    {
                        values.emplace_back( value );
                    }
                }
            }
        }

        if ( positionOfEndToken != std::string::npos )
        {
            isEndTokenKeywordRead = true;
            continue;
        }
    }

    return std::make_pair( keywordName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseKeywordContent> RifEclipseTextFileReader::parseStringData( const std::string_view& stringData )
{
    size_t offset    = 0;
    size_t bytesRead = 0;

    std::vector<RifEclipseKeywordContent> dataObjects;

    while ( offset < stringData.size() )
    {
        auto [keyword, values] = RifEclipseTextFileReader::readKeywordAndValues( stringData, offset, bytesRead );

        if ( !keyword.empty() )
        {
            RifEclipseKeywordContent content;
            content.keyword = keyword;
            content.offset  = offset;
            if ( values.empty() )
            {
                content.content = stringData.substr( offset, bytesRead );
            }
            else
            {
                content.values = values;
            }

            dataObjects.emplace_back( content );
        }

        offset += bytesRead;
    }

    return dataObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string_view RifEclipseTextFileReader::readLine( const std::string_view& source, const size_t offset, size_t& bytesRead )
{
    if ( offset >= source.size() ) return {};

    auto start = source.find_first_not_of( RifEclipseTextFileReader::m_whiteSpace, offset );
    if ( start == std::string::npos )
    {
        bytesRead = source.size() - offset;
        return source.substr( offset, bytesRead );
    }

    auto end = source.find_first_of( '\n', start );
    if ( end != std::string::npos )
    {
        // Add 1 to skip the \n we have found
        bytesRead = end - offset + 1;
        return source.substr( start, end - start );
    }

    // No line shift found, reached end of string data
    end = source.size();

    bytesRead = end - offset;
    return source.substr( start, bytesRead );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseTextFileReader::rtrim( std::string_view& s, const char* t /*= ws */ )
{
    if ( s.empty() ) return;

    s = s.substr( 0, s.find_last_not_of( t ) + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseTextFileReader::ltrim( std::string_view& s, const char* t /*= ws */ )
{
    if ( s.empty() ) return;

    s = s.substr( s.find_first_not_of( t ), s.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseTextFileReader::trim( std::string_view& s, const char* t /*= ws */ )
{
    if ( s.empty() ) return;

    rtrim( s, t );
    ltrim( s, t );
}
