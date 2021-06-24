/*
 * DISTRHO OneKnob Devil's Distortion
 * Based on Steve Harris Barry's Satan Maximizer
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 * Copyright (C) 2002-2003 <steve@plugin.org.uk>
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

#include "OneKnobDevilDistortionUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 320;

static const OneKnobMainControl main = {
    kParameterKneePoint,
    "Knee Point",
    "dB",
    -90.0f,
    0.0f,
    0.0f
};

static const char* const switchText = "How many samples...";

static const OneKnobAuxiliaryButtonGroupValue buttonGroupValues[] = {
    {  7,  "7" },
    { 15, "15" },
    { 24, "24" },
    { 30, "30" },
};

static const OneKnobAuxiliaryButtonGroup buttonGroup = {
    kParameterDecayTime,
    "How many samples...",
    sizeof(buttonGroupValues)/sizeof(buttonGroupValues[0]),
    buttonGroupValues
};

// --------------------------------------------------------------------------------------------------------------------

OneKnobDevilDistortionUI::OneKnobDevilDistortionUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // setup OneKnob UI
    const Rectangle<uint> mainArea(kSidePanelWidth,
                                   kDefaultHeight/4 - kSidePanelWidth,
                                   kDefaultWidth/2 - kSidePanelWidth,
                                   kDefaultHeight*5/8 - kSidePanelWidth);
    createMainControl(mainArea, main);

    const Rectangle<uint> buttonGroupArea(kDefaultWidth/2,
                                          kDefaultHeight/4,
                                          kDefaultWidth/2 - kSidePanelWidth,
                                          kDefaultHeight*3/4);
    createAuxiliaryButtonGroup(buttonGroupArea, buttonGroup);

    repositionWidgets();

    // set default values
    programLoaded(0);
}

// --------------------------------------------------------------------------------------------------------------------
// DSP Callbacks

void OneKnobDevilDistortionUI::parameterChanged(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterKneePoint:
        setMainControlValue(value);
        break;
    case kParameterDecayTime:
        setAuxiliaryButtonGroupValue(value);
        break;
    default:
        OneKnobUI::parameterChanged(index, value);
        break;
    }

    repaint();
}

void OneKnobDevilDistortionUI::programLoaded(const uint32_t index)
{
    switch (index)
    {
    case kOneKnobProgramDefault:
        setMainControlValue(kParameterDefaults[kParameterKneePoint]);
        setAuxiliaryButtonGroupValue(kParameterDefaults[kParameterDecayTime]);
        break;
    }

    repaint();
}

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobDevilDistortionUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
