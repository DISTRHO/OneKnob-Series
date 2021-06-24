/*
 * DISTRHO OneKnob BrickWall Limiter
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#pragma once

#include "OneKnobPluginInfo.h"

#define DISTRHO_PLUGIN_NAME         "OneKnob BrickWall Limiter"
#define DISTRHO_PLUGIN_URI          "https://kx.studio/Plugins:OneKnob#BrickWallLimiter"
#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:LimiterPlugin"

enum Parameters
{
    kParameterThreshold = 0,
    kParameterAutoGain,
    kParameterCount
};

enum Programs
{
    kProgramGentle,
    kProgramDestructive,
    kProgramInsane,
    kProgramCount
};

enum States
{
    kStateCount
};

static const float kParameterDefaults[kParameterCount] = {
    0.0f,
    0.0f
};

static const char* const kStateNames[kStateCount] = {
};

static const char* const kStateDefaults[kStateCount] = {
};
