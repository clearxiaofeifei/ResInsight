//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectCapability.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapabilityRegister.h"

#include <QString>

#include <map>
#include <memory>
#include <vector>

class QTextStream;

#define CAF_PDM_InitScriptableObject( uiName, ... )                                               \
    std::vector<QString> arguments = { __VA_ARGS__ };                                             \
    QString              iconResourceName;                                                        \
    QString              toolTip;                                                                 \
    QString              whatsThis;                                                               \
    if ( arguments.size() > 0 ) iconResourceName = arguments[0];                                  \
    if ( arguments.size() > 1 ) toolTip = arguments[1];                                           \
    if ( arguments.size() > 2 ) whatsThis = arguments[2];                                         \
                                                                                                  \
    CAF_PDM_InitObject( uiName, iconResourceName, toolTip, whatsThis );                           \
    caf::PdmObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( classKeyword(), \
                                                                                  classKeyword(), \
                                                                                  whatsThis );

#define CAF_PDM_InitScriptableObjectWithNameAndComment( uiName, iconResourceName, toolTip, whatsThis, scriptClassName, scriptComment ) \
    CAF_PDM_InitObject( uiName, iconResourceName, toolTip, whatsThis );                                                                \
    caf::PdmObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( classKeyword(),                                      \
                                                                                  scriptClassName,                                     \
                                                                                  scriptComment );

namespace caf
{
class PdmObject;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmScriptIOMessages;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmObjectScriptingCapability : public PdmObjectCapability
{
public:
    PdmObjectScriptingCapability( PdmObjectHandle* owner, bool giveOwnership );

    ~PdmObjectScriptingCapability() override;

    void readFields( QTextStream& inputStream, PdmObjectFactory* objectFactory, PdmScriptIOMessages* errorMessageContainer );
    void writeFields( QTextStream& outputStream ) const;

private:
    PdmObjectHandle* m_owner;
};
} // namespace caf
