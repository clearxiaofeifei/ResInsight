//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTimeEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QGridLayout>

#include <utility>

namespace caf
{
// Register default field editor for selected types
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiCheckBoxEditor, bool );

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, QString );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, int );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, double );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, float );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, quint64 );

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiDateEditor, QDate );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiDateEditor, QDateTime );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiTimeEditor, QTime );

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiTreeSelectionEditor, std::vector<QString> );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiTreeSelectionEditor, std::vector<int> );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiTreeSelectionEditor, std::vector<unsigned int> );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiTreeSelectionEditor, std::vector<float> );

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiFilePathEditor, FilePath );

// As the macro CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR() is not working correctly for std::pair<bool, double> the
// registration of this type in the factory has to be written directly
static bool myPdmUiCheckBoxAndTextEditor73 =
    caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance()->registerCreator<PdmUiCheckBoxAndTextEditor>(
        QString( typeid( caf::PdmField<std::pair<bool, double>> ).name() ) );
static bool my2PdmUiCheckBoxAndTextEditor73 =
    caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance()->registerCreator<PdmUiCheckBoxAndTextEditor>(
        QString( typeid( caf::PdmProxyValueField<std::pair<bool, double>> ).name() ) );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::PdmUiDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::~PdmUiDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDefaultObjectEditor::createWidget( QWidget* parent )
{
    QWidget* widget = new QWidget( parent );
    widget->setObjectName( "ObjectEditor" );
    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::recursivelyConfigureAndUpdateTopLevelUiOrdering( const PdmUiOrdering& topLevelUiOrdering,
                                                                                const QString&       uiConfigName )
{
    CAF_ASSERT( this->widget() );

    recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( topLevelUiOrdering, this->widget(), uiConfigName );
}

} // end namespace caf
