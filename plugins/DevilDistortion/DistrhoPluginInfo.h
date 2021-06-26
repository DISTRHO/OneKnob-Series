/*
 * DISTRHO OneKnob Devil's Distortion
 * Based on Steve Harris Barry's Satan Maximizer
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 * Copyright (C) 2002-2003 <steve@plugin.org.uk>
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

#define DISTRHO_PLUGIN_NAME         "OneKnob Devil's Distortion"
#define DISTRHO_PLUGIN_URI          "https://kx.studio/Plugins:OneKnob#DevilDistortion"
#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:DistortionPlugin"

enum Parameters
{
    kParameterKneePoint = 0,
    kParameterDecayTime,
    kParameterCount
};

enum Programs
{
    kProgramCount
};

enum States
{
    kStateCount
};

static const float kParameterDefaults[kParameterCount] = {
    0.0f,
    23.0f
};

static const char* const kStateNames[kStateCount] = {
};

static const char* const kStateDefaults[kStateCount] = {
};
