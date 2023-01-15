/*
 * DISTRHO OneKnob Devil's Distortion
 * Based on Steve Harris Barry's Satan Maximizer
 * Copyright (C) 2002-2003 <steve@plugin.org.uk>
 * Copyright (C) 2021-2023 Filipe Coelho <falktx@falktx.com>
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

#define DISTRHO_PLUGIN_NAME    "OneKnob Devil's Distortion"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#DevilDistortion"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.DevilDistortion"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "distortion", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:DistortionPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Distortion|Stereo"

enum Parameters {
    kParameterKneePoint = 0,
    kParameterDecayTime,
    kParameterBypass,
    kParameterCount
};

enum Programs {
    kProgramDefault,
    kProgramCount
};

enum States {
    kStateCount
};

static constexpr const struct OneKnobParameterRanges {
    float min, def, max;
} kParameterRanges[kParameterCount] = {
    { -90.f, 0.f, 0.f },
    { 2.f, 23.f, 30.f },
    {}
};
