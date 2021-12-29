/*
 * DISTRHO OneKnob A/B Output Selector
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

#include "OneKnobPlugin.hpp"
#include "LinearSmoother.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobOutputSelectorPlugin : public OneKnobPlugin
{
public:
    OneKnobOutputSelectorPlugin()
        : OneKnobPlugin()
    {
        init();
        sampleRateChanged(getSampleRate());
        abSmooth.setTimeConstant(1e-3f);
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
            parameter.ranges.def = kParameterDefaults[kParameterSelect];
            parameter.ranges.min = -100.0f;
            parameter.ranges.max = 100.0f;
            {
                ParameterEnumerationValue *values = new ParameterEnumerationValue[3];
                parameter.enumValues.values = values;
                parameter.enumValues.count = 3;
                parameter.enumValues.restrictedMode = false;
                values[0].value = -100.0f;
                values[0].label = "A";
                values[1].value = 0.0f;
                values[1].label = "A/B";
                values[2].value = 100.0f;
                values[2].label = "B";
            }
            break;
        case kParameterMode:
            parameter.hints      = kParameterIsAutomatable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name       = "Mode";
            parameter.symbol     = "mode";
            parameter.unit       = "";
            parameter.ranges.def = kParameterDefaults[kParameterMode];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
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

        // TODO do something extra here? if not, remove this
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

        abSmooth.setTarget((parameters[kParameterSelect] + 100.0f) / 200.0f);
        abSmooth.clearToTarget();
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* const li = inputs[0];
        const float* const ri = inputs[1];
        /* */ float* const l1 = outputs[0];
        /* */ float* const r1 = outputs[1];
        /* */ float* const l2 = outputs[2];
        /* */ float* const r2 = outputs[3];

        abSmooth.setTarget((parameters[kParameterSelect] + 100.0f) / 200.0f);

        float sel, As, Bs;
        for (uint32_t i = 0; i < frames; ++i)
        {
            sel = abSmooth.next();
            As = std::sqrt(1.0f - sel);
            Bs = std::sqrt(sel);
            l1[i] = As*li[i];
            r1[i] = As*ri[i];
            l2[i] = Bs*li[i];
            r2[i] = Bs*ri[i];

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
    LinearSmoother abSmooth;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobOutputSelectorPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobOutputSelectorPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
