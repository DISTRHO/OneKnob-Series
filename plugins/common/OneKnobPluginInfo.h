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

#include "FloatFifo.hpp"

// --------------------------------------------------------------------------------------------------------------------

#define DISTRHO_PLUGIN_BRAND "DISTRHO"

#define DISTRHO_PLUGIN_HAS_UI          1
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_NUM_INPUTS      2
#define DISTRHO_PLUGIN_NUM_OUTPUTS     2
#define DISTRHO_PLUGIN_WANT_PROGRAMS   1
#define DISTRHO_PLUGIN_WANT_STATE      1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 0

// --------------------------------------------------------------------------------------------------------------------

#ifdef __clang__
# define MATH_CONSTEXPR
#else
# define MATH_CONSTEXPR constexpr
#endif

// --------------------------------------------------------------------------------------------------------------------

typedef FloatFifo<32> OneKnobFloatFifo;
typedef FloatFifoControl<32> OneKnobFloatFifoControl;

struct OneKnobLineGraphFifos {
    OneKnobFloatFifo v1;
    OneKnobFloatFifo v2;
    bool closed;
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

struct OneKnobAuxiliaryFileButton {
    const char* label;
    const char* key;
};

// --------------------------------------------------------------------------------------------------------------------
