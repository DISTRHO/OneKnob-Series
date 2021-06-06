/*
 * DISTRHO OneKnob BrickWall Limiter
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

#include "OneKnobBrickWallLimiterUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 320;

static const OneKnobMainControl main = {
    kParameterThreshold,
    "Threshold",
    "dB",
    50.0f,
    1000.0f
};

static const OneKnobAuxiliaryCheckBox checkBox = {
    kParameterAutoGain,
    "Automatic Gain",
    "Something that explains BrickWall limiter here"
};

// --------------------------------------------------------------------------------------------------------------------

OneKnobBrickWallLimiterUI::OneKnobBrickWallLimiterUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // setup OneKnob UI
    const Rectangle<uint> mainArea(kSidePanelWidth,
                                   kDefaultHeight/4,
                                   kDefaultWidth/3 - kSidePanelWidth,
                                   kDefaultHeight/2);
    createMainControl(mainArea, main);

    const Rectangle<uint> checkBoxArea(kDefaultWidth*2/3,
                                       kDefaultHeight/4,
                                       kDefaultWidth/3 - kSidePanelWidth,
                                       kDefaultHeight/2);
    createAuxiliaryCheckBox(checkBoxArea, checkBox);

    repositionWidgets();

    // set default values
    programLoaded(0);
}

// --------------------------------------------------------------------------------------------------------------------
// DSP Callbacks

void OneKnobBrickWallLimiterUI::parameterChanged(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterThreshold:
        setMainControlValue(value);
        break;
    case kParameterAutoGain:
        setAuxiliaryCheckBoxValue(value);
        break;
    case kParameterLineUpdateTickL:
        // TESTING
        lines[lineWriteIndex] = value;
        if (++lineWriteIndex == sizeof(lines)/sizeof(lines[0]))
            lineWriteIndex = 0;
        break;
    case kParameterLineUpdateTickR:
        break;
    }

    repaint();
}

void OneKnobBrickWallLimiterUI::programLoaded(const uint32_t index)
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

void OneKnobBrickWallLimiterUI::stateChanged(const char*, const char*)
{
}

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobBrickWallLimiterUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
