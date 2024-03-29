/*
 * DISTRHO OneKnob Convolution Reverb
 * Copyright (C) 2022-2023 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "OneKnobUI.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kDefaultWidth = 640;
static const uint kDefaultHeight = 400;

static const OneKnobMainControl main = {
    kParameterWetLevel,
    "Wet Level",
    "dB",
};

static constexpr const OneKnobAuxiliaryCheckBox kHighPassCheckBox = {
    kParameterHighPassFilter,
    "High Pass Filter",
    "Activate to prevent low-band/bassy sounds from being sent to the reverb"
};

static const OneKnobAuxiliarySlider numFieldOpts = {
    kParameterDryLevel,
    "Dry Level",
    "dB",
};

static const OneKnobAuxiliaryFileButton fileButtonOpts = {
    "Open IR File...",
    "irfile"
};

static const char* kConvolutionLineMeterNames[2] = { "Dry:", "Wet:" };

// --------------------------------------------------------------------------------------------------------------------

class OneKnobConvolutionReverbUI : public OneKnobUI
{
public:
    OneKnobConvolutionReverbUI()
        : OneKnobUI(kDefaultWidth, kDefaultHeight, kConvolutionLineMeterNames)
    {
        // setup OneKnob UI
        const Rectangle<uint> mainArea(kSidePanelWidth,
                                       kDefaultHeight*3/16 - kSidePanelWidth,
                                       kDefaultWidth/2 - kSidePanelWidth,
                                       kDefaultHeight*9/16);
        createMainControl(mainArea, main);

        const Rectangle<uint> auxArea(kDefaultWidth/2,
                                      kDefaultHeight/4,
                                      kDefaultWidth/2 - kSidePanelWidth,
                                      kDefaultHeight*3/4);
        createAuxiliaryFileButton(auxArea, kHighPassCheckBox, numFieldOpts, fileButtonOpts);

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
        case kParameterWetLevel:
            setMainControlValue(value);
            break;
        case kParameterDryLevel:
            setAuxiliaryNumFieldValue(value);
            break;
        case kParameterHighPassFilter:
            setAuxiliaryCheckBoxValue(value);
            break;
        }

        repaint();
    }

    void programLoaded(const uint32_t index) override
    {
        switch (index)
        {
        case kProgramDefault:
            setMainControlValue(kParameterRanges[kParameterWetLevel].def);
            setAuxiliaryNumFieldValue(kParameterRanges[kParameterDryLevel].def);
            setAuxiliaryCheckBoxValue(kParameterRanges[kParameterHighPassFilter].def > 0.5f);
            break;
        }

        repaint();
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobConvolutionReverbUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new OneKnobConvolutionReverbUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
