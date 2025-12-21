/*
 * DISTRHO OneKnob Drummer
 * Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
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

#include <cstdint>

#include "OneKnobPluginInfo.h"

#define DISTRHO_PLUGIN_NAME    "OneKnob Drummer"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#Drummer"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.Drummer"

#define DISTRHO_PLUGIN_WANT_TIMEPOS 1

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "generator", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:GeneratorPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Generator|Stereo"

enum Parameters {
    kParameterBypass,
    kParameterStyle,
    kParameterActive,
    kParameterFillIn,
    kParameterNext,
    kParameterStatus,
    kParameterCount
};

enum Programs {
    kProgramDefault,
    kProgramCount
};

enum States {
    kStateCount
};

static constexpr const uint32_t kDefaultStyle = 12;
static constexpr const uint32_t kNumStyles = 17;

static constexpr const char* const kStyleNames[kNumStyles] = {
    "Blues",
    "Blues Jump",
    "Blues Shuffle",
    "Blues Slow",
    "Funk",
    "Jazz Ballad",
    "Jazz Jump Swing",
    "Jazz Light Shuffle",
    "Jazz Shuffle",
    "Jazz Swing",
    "Pop",
    "Pop Ballad",
    "Rock",
    "Rock 2",
    "Rock 3",
    "Rock 4",
    "Rock Ballad",
};

static constexpr const struct OneKnobParameterRanges {
    float min, def, max;
} kParameterRanges[kParameterCount] = {
    {},
    { 0.f, kDefaultStyle, kNumStyles - 1 },
    { 0.f, 0.f, 1.f },
    { 0.f, 0.f, 1.f },
    { 0.f, 0.f, 1.f },
    { 0.f, 0.f, 5.f }
};
