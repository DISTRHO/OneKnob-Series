/*
 * DISTRHO OneKnob Sampler
 * Based on DSSI Simple Sampler
 * Copyright (C) 2008 Chris Cannam <cannam@all-day-breakfast.com>
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

#define DISTRHO_PLUGIN_NAME    "OneKnob Sampler"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#Sampler"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.Sampler"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "instrument", "sampler", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:GeneratorPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Instrument|Sampler|Stereo"

#undef DISTRHO_PLUGIN_NUM_INPUTS
#define DISTRHO_PLUGIN_NUM_INPUTS 0

#define DISTRHO_PLUGIN_IS_SYNTH 1

enum Parameters {
    kParameterRetune,
    kParameterBasePitch,
    kParameterSustain,
    kParameterReleaseTime,
    kParameterBalance,
    kParameterBypass,
    kParameterCount
};

enum Programs {
    kProgramInit,
    kProgramCount
};

enum States {
    kStateFile,
    kStateCount
};

static constexpr const struct OneKnobParameterRanges {
    float min, def, max;
} kParameterRanges[kParameterCount] = {
    { 0.f, 1.f, 1.f },
    { 0.f, 60.f, 127.f },
    { 0.f, 0.f, 1.f },
    { 0.001f, 0.001f, 2.f },
    { -100.f, 0.f, 100.f },
    {}
};
