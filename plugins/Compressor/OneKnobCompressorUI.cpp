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

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 320;

static const OneKnobMainControl main = {
    kParameterRelease,
    "Release",
    "ms",
    50.0f,
    1000.0f,
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

OneKnobCompressorUI::OneKnobCompressorUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // setup OneKnob UI
    const Rectangle<uint> mainArea(kSidePanelWidth,
                                   kDefaultHeight/4 - kSidePanelWidth,
                                   kDefaultWidth/2 - kSidePanelWidth,
                                   kDefaultHeight*5/8 - kSidePanelWidth);
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

// --------------------------------------------------------------------------------------------------------------------
// DSP Callbacks

void OneKnobCompressorUI::parameterChanged(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterRelease:
        setMainControlValue(value);
        break;
    case kParameterMode:
        setAuxiliaryComboBoxValue(value);
        break;
    default:
        OneKnobUI::parameterChanged(index, value);
        break;
    }

    repaint();
}

void OneKnobCompressorUI::programLoaded(const uint32_t index)
{
    switch (index)
    {
    case kOneKnobProgramDefault:
        setMainControlValue(kParameterDefaults[kParameterRelease]);
        setAuxiliaryComboBoxValue(kParameterDefaults[kParameterMode]);
        break;
    case kOneKnobBaseProgramCount + kProgramConservative:
        break;
    case kOneKnobBaseProgramCount + kProgramLiberal:
        break;
    case kOneKnobBaseProgramCount + kProgramExtreme:
        break;
    }

    repaint();
}

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobCompressorUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
