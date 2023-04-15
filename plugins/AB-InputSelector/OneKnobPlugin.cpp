/*
 * DISTRHO OneKnob A/B Input Selector
 * Copyright (C) 2021-2023 Filipe Coelho <falktx@falktx.com>
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

#include "OneKnobPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobInputSelectorPlugin : public OneKnobPlugin
{
public:
    OneKnobInputSelectorPlugin()
        : OneKnobPlugin()
    {
        abSmooth.setTimeConstant(1e-3f);
        abSmooth.setTargetValue((kParameterRanges[kParameterSelect].def + 100.0f) / 200.0f);

        init();
        sampleRateChanged(getSampleRate());
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        // TODO
        return "...";
    }

    const char* getLicense() const noexcept override
    {
        // TODO
        return "LGPL";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'i', 's');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(const uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterSelect:
            parameter.hints      = kParameterIsAutomatable;
            parameter.name       = "A/B Select";
            parameter.symbol     = "select";
            parameter.unit       = "%";
            parameter.ranges.def = kParameterRanges[kParameterSelect].def;
            parameter.ranges.min = kParameterRanges[kParameterSelect].min;
            parameter.ranges.max = kParameterRanges[kParameterSelect].max;
            {
                ParameterEnumerationValue *values = new ParameterEnumerationValue[3];
                parameter.enumValues.values = values;
                parameter.enumValues.count = 3;
                parameter.enumValues.restrictedMode = false;
                values[0].value = kParameterRanges[kParameterSelect].min;
                values[0].label = "A";
                values[1].value = kParameterRanges[kParameterSelect].def;
                values[1].label = "A/B";
                values[2].value = kParameterRanges[kParameterSelect].max;
                values[2].label = "B";
            }
            break;
        case kParameterMode:
            parameter.hints      = kParameterIsAutomatable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name       = "Mode";
            parameter.symbol     = "mode";
            parameter.unit       = "";
            parameter.ranges.def = kParameterRanges[kParameterMode].def;
            parameter.ranges.min = kParameterRanges[kParameterMode].min;
            parameter.ranges.max = kParameterRanges[kParameterMode].max;
            break;
        case kParameterBypass:
            parameter.initDesignation(kParameterDesignationBypass);
            break;
        }
    }

    void initProgramName(uint32_t index, String& programName) override
    {
        switch (index)
        {
        case kProgramInit:
            programName = "Init";
            break;
        }
    }

    // -------------------------------------------------------------------
    // Internal data

    void setParameterValue(const uint32_t index, const float value) override
    {
        OneKnobPlugin::setParameterValue(index, value);

        if (index == kParameterSelect)
            abSmooth.setTargetValue((value + 100.0f) / 200.0f);
    }

    void loadProgram(const uint32_t index) override
    {
        switch (index)
        {
        case kProgramInit:
            loadDefaultParameterValues();
            break;
        }
    }

    // -------------------------------------------------------------------
    // Process

    void sampleRateChanged(const double newSampleRate) override
    {
        OneKnobPlugin::sampleRateChanged(newSampleRate);

        abSmooth.setSampleRate((float)newSampleRate);
    }

    void activate() override
    {
        OneKnobPlugin::activate();

        abSmooth.clearToTargetValue();
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* const l1 = inputs[0];
        const float* const r1 = inputs[1];
        const float* const l2 = inputs[2];
        const float* const r2 = inputs[3];
        /* */ float* const lo = outputs[0];
        /* */ float* const ro = outputs[1];

        float sel, As, Bs;
        for (uint32_t i = 0; i < frames; ++i)
        {
            sel = abSmooth.next();
            As = std::sqrt(1.0f - sel);
            Bs = std::sqrt(sel);
            lo[i] = As * l1[i]+Bs * l2[i];
            ro[i] = As * r1[i]+Bs * r2[i];

            lineGraphHighest1 = std::max(lineGraphHighest1, std::abs(l1[i]));
            lineGraphHighest2 = std::max(lineGraphHighest2, std::abs(l2[i]));

            if (++lineGraphFrameCounter == lineGraphFrameToReset)
            {
                lineGraphFrameCounter = 0;
                setMeters(lineGraphHighest1, lineGraphHighest2);
                lineGraphHighest1 = lineGraphHighest2 = 0.0f;
            }
        }
    }

    // -------------------------------------------------------------------
    LinearValueSmoother abSmooth;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobInputSelectorPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobInputSelectorPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
