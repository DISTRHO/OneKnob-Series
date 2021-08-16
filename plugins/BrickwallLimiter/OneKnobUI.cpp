/*
 * DISTRHO OneKnob Brickwall Limiter
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

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "OneKnobUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 400;

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

class OneKnobBrickwallLimiterUI : public OneKnobUI
{
public:
    OneKnobBrickwallLimiterUI()
        : OneKnobUI(kDefaultWidth, kDefaultHeight)
    {
        // setup OneKnob UI
        const Rectangle<uint> mainArea(kSidePanelWidth,
                                       kDefaultHeight*3/16 - kSidePanelWidth,
                                       kDefaultWidth/2 - kSidePanelWidth,
                                       kDefaultHeight*9/16);
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

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(const uint32_t index, const float value) override
    {
        switch (index)
        {
        case kParameterThreshold:
            setMainControlValue(value);
            break;
        case kParameterAutoGain:
            setAuxiliaryCheckBoxValue(value);
            break;
        }

        repaint();
    }

    void programLoaded(const uint32_t index) override
    {
        switch (index)
        {
        case kProgramInit:
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

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobBrickwallLimiterUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobBrickwallLimiterUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
