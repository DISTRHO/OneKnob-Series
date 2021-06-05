/*
 * DISTRHO OneKnob Compressor
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * For a full copy of the license see the LICENSE file.
 */

#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND "DISTRHO"
#define DISTRHO_PLUGIN_NAME  "OneKnob Compressor"
#define DISTRHO_PLUGIN_URI   "https://kx.studio/Plugins:OneKnob#Compressor"

#define DISTRHO_PLUGIN_HAS_UI        1
#define DISTRHO_PLUGIN_IS_RT_SAFE    1
#define DISTRHO_PLUGIN_NUM_INPUTS    2
#define DISTRHO_PLUGIN_NUM_OUTPUTS   2
#define DISTRHO_PLUGIN_WANT_PROGRAMS 1
#define DISTRHO_PLUGIN_WANT_STATE    1

#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:CompressorPlugin"

enum Parameters
{
    kParameterRelease = 0,
    kParameterMode,
    kParameterLineUpdateTickL,
    kParameterLineUpdateTickR,
    kParameterCount
};

enum Programs
{
    kProgramDefault = 0,
    kProgramConservative,
    kProgramLiberal,
    kProgramExtreme,
    kProgramCount
};

enum States
{
    kStateScaleFactor = 0,
    kStateLineUpdateTime,
    kStateCount
};

static const float kParameterDefaults[kParameterCount] = {
    100.0f,
    2.0f,
    0.0f,
    0.0f
};

static const char* const kStateNames[kStateCount] = {
    "ScaleFactor",
    "LineUpdateTime"
};

static const char* const kStateDefaults[kStateCount] = {
    "1.0",
    "5000"
};

#endif // DISTRHO_PLUGIN_INFO_H_INCLUDED
