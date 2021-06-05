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

#include "OneKnobCompressorUI.hpp"

START_NAMESPACE_DISTRHO

static const uint kDefaultWidth = 512;
static const uint kDefaultHeight = 380;

// -----------------------------------------------------------------------

OneKnobCompressorUI::OneKnobCompressorUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // set default values
    programLoaded(0);
}

// -----------------------------------------------------------------------
// DSP Callbacks

void OneKnobCompressorUI::parameterChanged(uint32_t index, float value)
{
    switch (index)
    {
    case kParameterRelease:
        break;
    case kParameterMode:
        break;
    case kParameterLineUpdateTickL:
        break;
    case kParameterLineUpdateTickR:
        break;
    }
}

void OneKnobCompressorUI::programLoaded(uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        break;
    case kProgramConservative:
        break;
    case kProgramLiberal:
        break;
    case kProgramExtreme:
        break;
    }
}

void OneKnobCompressorUI::stateChanged(const char* key, const char* value)
{
}

// -----------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobCompressorUI();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
