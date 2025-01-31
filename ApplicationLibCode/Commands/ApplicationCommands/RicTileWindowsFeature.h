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

#include "RiaPlotDefines.h"

#include "cafCmdFeature.h"

class RiuMainWindow;
class RiuPlotMainWindow;

//==================================================================================================
///
//==================================================================================================
class RicTileWindowsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void applyTiling( RiuMainWindow* mainWindow, RiaDefines::WindowTileMode requestedTileMode );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};

//==================================================================================================
///
//==================================================================================================
class RicTileWindowsVerticallyFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};

//==================================================================================================
///
//==================================================================================================
class RicTileWindowsHorizontallyFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};

//==================================================================================================
///
//==================================================================================================
class RicTilePlotWindowsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void applyTiling( RiuPlotMainWindow* mainWindow, RiaDefines::WindowTileMode requestedTileMode );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};

//==================================================================================================
///
//==================================================================================================
class RicTilePlotWindowsVerticallyFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};

//==================================================================================================
///
//==================================================================================================
class RicTilePlotWindowsHorizontallyFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandChecked() override;
};
