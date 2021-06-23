/*
 * DISTRHO OneKnob Brickwall Limiter
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

#include "OneKnobBrickwallLimiterUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 320;

static const OneKnobMainControl main = {
    kParameterThreshold,
    "Threshold",
    "dB",
    -50.0f,
    0.0f,
    0.0f
};

static const OneKnobAuxiliaryCheckBox checkBox = {
    kParameterAutoGain,
    "Automatic Gain",
    "Activate to compensate for any perceived loudness lost.\nWinning the loudness wars!"
};

// --------------------------------------------------------------------------------------------------------------------

OneKnobBrickwallLimiterUI::OneKnobBrickwallLimiterUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // setup OneKnob UI
    const Rectangle<uint> mainArea(kSidePanelWidth,
                                   kDefaultHeight/4 - kSidePanelWidth,
                                   kDefaultWidth/2 - kSidePanelWidth,
                                   kDefaultHeight*5/8 - kSidePanelWidth);
    createMainControl(mainArea, main);

    const Rectangle<uint> checkBoxArea(kDefaultWidth/2,
                                       kDefaultHeight/4,
                                       kDefaultWidth/2 - kSidePanelWidth,
                                       kDefaultHeight*3/4);
    createAuxiliaryCheckBox(checkBoxArea, checkBox);

    repositionWidgets();

    // set default values
    programLoaded(0);
}

// --------------------------------------------------------------------------------------------------------------------
// DSP Callbacks

void OneKnobBrickwallLimiterUI::parameterChanged(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterThreshold:
        setMainControlValue(value);
        break;
    case kParameterAutoGain:
        setAuxiliaryCheckBoxValue(value);
        break;
    case kParameterLineUpdateTickIn:
        pushInputMeter(std::abs(value));
        break;
    case kParameterLineUpdateTickOut:
        pushOutputMeter(std::abs(value));
        break;
    }

    repaint();
}

void OneKnobBrickwallLimiterUI::programLoaded(const uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        setMainControlValue(kParameterDefaults[kParameterThreshold]);
        setAuxiliaryCheckBoxValue(kParameterDefaults[kParameterAutoGain]);
        break;
    case kProgramGentle:
        break;
    case kProgramDestructive:
        break;
    case kProgramInsane:
        break;
    }

    repaint();
}

void OneKnobBrickwallLimiterUI::stateChanged(const char*, const char*)
{
}

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobBrickwallLimiterUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
