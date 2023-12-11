/*
 * DISTRHO OneKnob Filter
 * Copyright (C) 2023 Filipe Coelho <falktx@falktx.com>
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
#include "Biquad.h"

#define DISTRHO_PLUGIN_NAME    "OneKnob Filter"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#Filter"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.Filter"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "filter", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:FilterPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Filter|Stereo"

#ifdef __MOD_DEVICES__
#undef DISTRHO_PLUGIN_NUM_INPUTS
#undef DISTRHO_PLUGIN_NUM_OUTPUTS
#define DISTRHO_PLUGIN_NUM_INPUTS 1
#define DISTRHO_PLUGIN_NUM_OUTPUTS 1
#endif

enum Parameters {
    kParameterType = 0,
    kParameterFrequency,
    kParameterQ,
    kParameterGain,
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
    { bq_type_lowpass, bq_type_lowpass, bq_type_highshelf },
    { 20.f, 5000.f, 20000.f },
    { 0.f, 0.707f, 1.f },
    { -20.f, 0.f, 20.f },
    {}
};
