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

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 320;

static const OneKnobMainControl main = {
    "Release",
    "ms",
    0.0f,
    5000.0f
};

static const OneKnobAuxiliaryOptionValue values[] = {
    {
        0, "Off", "Nothing here"
    },
    {
        1, "Low", "Text to place for Low things"
    },
    {
        2, "Mid", "Text to place for Mid things"
    },
    {
        3, "High", "Text to place for High things"
    },
};

static const OneKnobAuxiliaryOption option = {
    "Mode",
    sizeof(values)/sizeof(values[0]),
    values
};

// -----------------------------------------------------------------------

OneKnobCompressorUI::OneKnobCompressorUI()
    : OneKnobUI(kDefaultWidth, kDefaultHeight)
{
    // setup OneKnob UI
    const Rectangle<uint> mainArea(kSidePanelWidth,
                                   kDefaultHeight/4,
                                   kDefaultWidth/3 - kSidePanelWidth,
                                   kDefaultHeight/2);
    createMainControl(mainArea, main);

    const Rectangle<uint> optionArea(kDefaultWidth*2/3,
                                     kDefaultHeight/4,
                                     kDefaultWidth/3 - kSidePanelWidth,
                                     kDefaultHeight/2);
    createAuxiliaryOption(optionArea, option);

    repositionWidgets();

    // set default values
    programLoaded(0);
}

// -----------------------------------------------------------------------
// DSP Callbacks

void OneKnobCompressorUI::parameterChanged(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterRelease:
        setMainControlValue(value);
        break;
    case kParameterMode:
        break;
    case kParameterLineUpdateTickL:
        break;
    case kParameterLineUpdateTickR:
        break;
    }

    repaint();
}

void OneKnobCompressorUI::programLoaded(const uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        setMainControlValue(kParameterDefaults[kParameterRelease]);
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

void OneKnobCompressorUI::stateChanged(const char*, const char*)
{
}

// -----------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobCompressorUI();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
