/*
 * DISTRHO OneKnob Compressor
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
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

#define DISTRHO_PLUGIN_NAME    "OneKnob Compressor"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#Compressor"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.Compressor"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "compressor", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:CompressorPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Dynamics|Stereo"

enum Parameters {
    kParameterRelease = 0,
    kParameterMode,
    kParameterCount
};

enum Programs {
    kProgramDefault,
    kProgramConservative,
    kProgramLiberal,
    kProgramExtreme,
    kProgramCount
};

enum States {
    kStateCount
};

static const float kParameterDefaults[kParameterCount] = {
    100.0f,
    2.0f
};
