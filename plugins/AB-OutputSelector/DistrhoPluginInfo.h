/*
 * DISTRHO OneKnob A/B Output Selector
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

#define DISTRHO_PLUGIN_NAME    "OneKnob A/B Output Selector"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#AB-OutputSelector"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.AB-OutputSelector"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "utility", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:UtilityPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Tools|Stereo"

#undef DISTRHO_PLUGIN_NUM_OUTPUTS
#define DISTRHO_PLUGIN_NUM_OUTPUTS 4

enum Parameters {
    kParameterSelect,
    kParameterMode,
    kParameterBypass,
    kParameterCount
};

enum Programs {
    kProgramInit,
    kProgramCount
};

enum States {
    kStateCount
};

static constexpr const struct OneKnobParameterRanges {
    float min, def, max;
} kParameterRanges[kParameterCount] = {
    { -100.f, 0.f, 100.f },
    { 0.f, 0.f, 1.f },
    {}
};
