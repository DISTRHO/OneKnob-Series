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

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "OneKnobUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 400;

static const OneKnobMainControl main = {
    kParameterRelease,
    "Release",
    "ms",
    50.0f,
    500.0f,
    100.0f
};

static const OneKnobAuxiliaryComboBoxValue comboBoxValues[] = {
    {
        0, "Off", "Signal pass-through"
    },
    {
        1, "Light", "Threshold: -12dB\nKnee: -12dB\nRatio: 2dB\nAttack: 10ms\nMakeup: -3dB"
    },
    {
        2, "Mild", "Threshold: -12dB\nKnee: -12dB\nRatio: 2dB\nAttack: 10ms\nMakeup: -3dB"
    },
    {
        3, "Heavy", "Threshold: -15dB\nKnee: -15dB\nRatio: 2dB\nAttack: 10ms\nMakeup: -3dB"
    },
};

static const OneKnobAuxiliaryComboBox comboBox = {
    kParameterMode,
    "Mode",
    sizeof(comboBoxValues)/sizeof(comboBoxValues[0]),
    comboBoxValues
};

// --------------------------------------------------------------------------------------------------------------------

class OneKnobCompressorUI : public OneKnobUI
{
public:
    OneKnobCompressorUI()
        : OneKnobUI(kDefaultWidth, kDefaultHeight)
    {
        // setup OneKnob UI
        const Rectangle<uint> mainArea(kSidePanelWidth,
                                       kDefaultHeight*3/16 - kSidePanelWidth,
                                       kDefaultWidth/2 - kSidePanelWidth,
                                       kDefaultHeight*9/16);
        createMainControl(mainArea, main);

        const Rectangle<uint> comboBoxArea(kDefaultWidth/2,
                                           kDefaultHeight/4,
                                           kDefaultWidth/2 - kSidePanelWidth,
                                           kDefaultHeight*3/4);
        createAuxiliaryComboBox(comboBoxArea, comboBox);

        repositionWidgets();

        // set default values
        programLoaded(0);
    }

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override
    {
        switch (index)
        {
        case kParameterRelease:
            setMainControlValue(value);
            break;
        case kParameterMode:
            setAuxiliaryComboBoxValue(value);
            break;
        }

        repaint();
    }

    void programLoaded(uint32_t index) override
    {
        switch (index)
        {
        case kProgramDefault:
            setMainControlValue(kParameterDefaults[kParameterRelease]);
            setAuxiliaryComboBoxValue(kParameterDefaults[kParameterMode]);
            break;
        case kProgramConservative:
            break;
        case kProgramLiberal:
            break;
        case kProgramExtreme:
            break;
        }

        repaint();
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobCompressorUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobCompressorUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
