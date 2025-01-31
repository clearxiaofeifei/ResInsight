/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaTextStringTools.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaTextStringTools::compare( const QString& expected, const QString& actual )
{
    // Suggestions for improvement
    // 1. report line number for first change
    // 2. report line numbers for all changes
    // 3. add support for compare with content of a text file on disk

    if ( expected.compare( actual ) == 0 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::trimAndRemoveDoubleSpaces( const QString& s )
{
    int     length;
    QString trimmed = s.trimmed();

    do
    {
        length  = trimmed.size();
        trimmed = trimmed.replace( "  ", " " );
    } while ( trimmed.size() < length );

    return trimmed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::commonRoot( const QStringList& stringList )
{
    QString root;
    if ( !stringList.isEmpty() )
    {
        root = stringList.front();
        for ( const auto& item : stringList )
        {
            if ( root.length() > item.length() )
            {
                root.truncate( item.length() );
            }

            for ( int i = 0; i < root.length(); ++i )
            {
                if ( root[i] != item[i] )
                {
                    root.truncate( i );
                    break;
                }
            }
        }
    }
    return root;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::commonSuffix( const QStringList& stringList )
{
    QString suffix;
    if ( !stringList.isEmpty() )
    {
        suffix = stringList.back();
        for ( const auto& item : stringList )
        {
            if ( suffix.length() > item.length() )
            {
                suffix = suffix.right( item.length() );
            }

            for ( int i = 0; i < suffix.length(); i++ )
            {
                int suffixIndex = suffix.length() - i - 1;
                int itemIndex   = item.length() - i - 1;
                if ( suffix[suffixIndex] != item[itemIndex] )
                {
                    suffix = suffix.right( i );
                    break;
                }
            }
        }
    }
    return suffix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::trimNonAlphaNumericCharacters( const QString& s )
{
    QString            trimmedString = s;
    QRegularExpression trimRe( "[^a-zA-Z0-9]+$" );
    trimmedString.replace( trimRe, "" );
    return trimmedString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitSkipEmptyParts( const QString& text, const QString& sep /*= " " */ )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 14, 0 )
    return text.split( sep, Qt::SkipEmptyParts, Qt::CaseInsensitive );
#else
    return text.split( sep, QString::SkipEmptyParts, Qt::CaseInsensitive );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::replaceTemplateTextWithValues( const QString& templateText, const std::map<QString, QString>& valueMap )
{
    QString resolvedText = templateText;

    // Use a regular expression to find all occurrences of ${key} in the text and replace with the value

    for ( const auto& [key, value] : valueMap )
    {
        QString regexString = key;
        regexString.replace( "$", "\\$" );
        regexString += "\\b";

        QRegularExpression rx( regexString );

        resolvedText.replace( rx, value );
    }

    return resolvedText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitSkipEmptyParts( const QString& text, const QRegExp& regExp )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 14, 0 )
    return text.split( regExp, Qt::SkipEmptyParts );
#else
    return text.split( regExp, QString::SkipEmptyParts );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::strong_ordering operator<=>( const QString& lhs, const QString& rhs )
{
    return lhs.compare( rhs ) <=> 0;
}
