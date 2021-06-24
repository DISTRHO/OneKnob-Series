/*
 * DISTRHO OneKnob Series
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

typedef unsigned int uint;

// --------------------------------------------------------------------------------------------------------------------

#define DISTRHO_PLUGIN_BRAND "DISTRHO"

#define DISTRHO_PLUGIN_HAS_UI        1
#define DISTRHO_PLUGIN_IS_RT_SAFE    1
#define DISTRHO_PLUGIN_NUM_INPUTS    2
#define DISTRHO_PLUGIN_NUM_OUTPUTS   2
#define DISTRHO_PLUGIN_WANT_PROGRAMS 1
#define DISTRHO_PLUGIN_WANT_STATE    1

// --------------------------------------------------------------------------------------------------------------------

enum OneKnobBaseParameters
{
    kOneKnobParameterLineUpdateTickIn,
    kOneKnobParameterLineUpdateTickOut,
    kOneKnobBaseParameterCount
};

enum OneKnobBasePrograms
{
    kOneKnobProgramDefault = 0,
    kOneKnobBaseProgramCount
};

enum OneKnobBaseStates
{
    kOneKnobStateScaleFactor = 0,
    kOneKnobStateLineUpdateTime,
    kOneKnobBaseStateCount
};

// --------------------------------------------------------------------------------------------------------------------

static const float kOneKnobBaseParameterDefaults[kOneKnobBaseParameterCount] = {
    0.0f,
    0.0f
};

static const char* const kOneKnobBaseStateNames[kOneKnobBaseStateCount] = {
    "ScaleFactor",
    "LineUpdateTime"
};

static const char* const kOneKnobBaseStateDefaults[kOneKnobBaseStateCount] = {
    "1.0",
    "5000"
};

// --------------------------------------------------------------------------------------------------------------------

struct OneKnobMainControl {
    uint id;
    const char* label;
    const char* unit;
    float min, max, def;
};

struct OneKnobAuxiliaryButtonGroupValue {
    int value;
    const char* label;
};

struct OneKnobAuxiliaryButtonGroup {
    uint id;
    const char* description;
    uint count;
    const OneKnobAuxiliaryButtonGroupValue* values;
};

struct OneKnobAuxiliaryCheckBox {
    uint id;
    const char* title;
    const char* description;
};

struct OneKnobAuxiliaryComboBoxValue {
    int value;
    const char* label;
    const char* description;
};

struct OneKnobAuxiliaryComboBox {
    uint id;
    const char* title;
    uint count;
    const OneKnobAuxiliaryComboBoxValue* values;
};

// --------------------------------------------------------------------------------------------------------------------
