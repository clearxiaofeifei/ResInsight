//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "caf.h"

#include "QtGui/qevent.h"
#include <QLocale>
#include <QTextStream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLocale norwegianLocale()
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    return QLocale::NorwegianBokmal;
#else
    return QLocale::Norwegian;
#endif
} // namespace caf::norwegianLocale()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& endl( QTextStream& s )
{
    // https: // github.com/qt/qtbase/blob/dev/src/corelib/serialization/qtextstream.cpp#L2845
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    return s << QLatin1Char( '\n' ) << Qt::flush;
#else
    return s << QLatin1Char( '\n' ) << flush;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPointF position( QWheelEvent* wheelEvent )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    return wheelEvent->position();
#else
    return wheelEvent->pos();
#endif
}

} // namespace caf
