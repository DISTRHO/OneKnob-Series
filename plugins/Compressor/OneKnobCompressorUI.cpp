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

namespace Art = DistrhoArtwork3BandEQ;

// -----------------------------------------------------------------------

OneKnobCompressorUI::OneKnobCompressorUI()
    : UI(Art::backgroundWidth, Art::backgroundHeight)
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
    case DistrhoPlugin3BandEQ::paramLow:
        fSliderLow->setValue(value);
        break;
    case DistrhoPlugin3BandEQ::paramMid:
        fSliderMid->setValue(value);
        break;
    case DistrhoPlugin3BandEQ::paramHigh:
        fSliderHigh->setValue(value);
        break;
    case DistrhoPlugin3BandEQ::paramMaster:
        fSliderMaster->setValue(value);
        break;
    case DistrhoPlugin3BandEQ::paramLowMidFreq:
        fKnobLowMid->setValue(value);
        break;
    case DistrhoPlugin3BandEQ::paramMidHighFreq:
        fKnobMidHigh->setValue(value);
        break;
    }
}

void OneKnobCompressorUI::programLoaded(uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        parameters[kParameterRelease] = kParameterDefaultRelease;
        parameters[kParameterMode] = kParameterDefaultMode;
        break;
    case kProgramConservative:
        parameters[kParameterRelease] = 100.0f;
        parameters[kParameterMode] = 1.0f;
        break;
    case kProgramLiberal:
        parameters[kParameterRelease] = 100.0f;
        parameters[kParameterMode] = 2.0f;
        break;
    case kProgramExtreme:
        parameters[kParameterRelease] = 100.0f;
        parameters[kParameterMode] = 3.0f;
        break;
    }
}

// -----------------------------------------------------------------------
// Widget Callbacks

void OneKnobCompressorUI::onDisplay()
{
    fImgBackground.draw();
}

// -----------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobCompressorUI();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
