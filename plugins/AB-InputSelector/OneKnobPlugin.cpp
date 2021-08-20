/*
 * DISTRHO OneKnob A/B Input Selector
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

class OneKnobInputSelectorPlugin : public OneKnobPlugin
{
public:
    OneKnobInputSelectorPlugin()
        : OneKnobPlugin()
    {
        init();
        sampleRateChanged(getSampleRate());
        fABSmooth.setTimeConstant(1e-3f);
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
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "A/B Select";
            parameter.symbol     = "select";
            parameter.unit       = "%";
            parameter.ranges.def = kParameterDefaults[kParameterSelect];
            parameter.ranges.min = -100.0f;
            parameter.ranges.max = 100.0f;
            break;
        case kParameterMode:
            parameter.hints      = kParameterIsAutomable | kParameterIsInteger | kParameterIsBoolean;
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

        fABSmooth.setSampleRate((float)newSampleRate);
    }

    void activate() override
    {
        OneKnobPlugin::activate();

        LinearSmoother &abSmooth = fABSmooth;
        abSmooth.setTarget((parameters[kParameterSelect] + 100.0f) / 200.0f);
        abSmooth.clearToTarget();
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float *l1 = inputs[0];
        const float *r1 = inputs[1];
        const float *l2 = inputs[2];
        const float *r2 = inputs[3];
        float *lo = outputs[0];
        float *ro = outputs[1];

        LinearSmoother &abSmooth = fABSmooth;
        abSmooth.setTarget((parameters[kParameterSelect] + 100.0f) / 200.0f);

        for (uint32_t i = 0; i < frames; ++i) {
            float sel = fABSmooth.next();
            float As = std::sqrt(1.0f - sel);
            float Bs = std::sqrt(sel);
            lo[i] = As * l1[i]+Bs * l2[i];
            ro[i] = As * r1[i]+Bs * r2[i];
        }
    }

    // -------------------------------------------------------------------
    LinearSmoother fABSmooth;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobInputSelectorPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobInputSelectorPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
