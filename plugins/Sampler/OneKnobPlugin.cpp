/*
 * DISTRHO OneKnob Sampler
 * Based on DSSI Simple Sampler
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 * Copyright (C) 2008 Chris Cannam <cannam@all-day-breakfast.com>
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

class OneKnobSamplerPlugin : public OneKnobPlugin
{
public:
    OneKnobSamplerPlugin()
        : OneKnobPlugin()
    {
        init();
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
        return "GPLv2+";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 's', 'a');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(const uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterRetune:
            parameter.hints      = kParameterIsAutomable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name       = "Retune";
            parameter.symbol     = "retune";
            parameter.unit       = "";
            parameter.midiCC     = 12;
            parameter.ranges.def = kParameterDefaults[index];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;
        case kParameterBasePitch:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Base Pitch";
            parameter.symbol     = "basepitch";
            parameter.unit       = "midiNote";
            parameter.midiCC     = 13;
            parameter.ranges.def = kParameterDefaults[index];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 127.0f;
            break;
        case kParameterSustain:
            parameter.hints      = kParameterIsAutomable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name       = "Sustain";
            parameter.symbol     = "sustain";
            parameter.unit       = "";
            parameter.midiCC     = 64;
            parameter.ranges.def = kParameterDefaults[index];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;
        case kParameterReleaseTime:
            parameter.hints      = kParameterIsAutomable | kParameterIsLogarithmic;
            parameter.name       = "Release Time";
            parameter.symbol     = "release";
            parameter.unit       = "s";
            parameter.midiCC     = 72;
            parameter.ranges.def = kParameterDefaults[index];
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 2.0f;
            break;
        case kParameterBalance:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Balance";
            parameter.symbol     = "balance";
            parameter.unit       = "%";
            parameter.midiCC     = 10;
            parameter.ranges.def = kParameterDefaults[index];
            parameter.ranges.min = -100.0f;
            parameter.ranges.max = 100.0f;
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

    void run(const float** const inputs, float** const outputs, const uint32_t frames,
             const MidiEvent* const midiEvents, const uint32_t midiEventCount) override
    {
        float* const out1 = outputs[0];
        float* const out2 = outputs[1];

        for (uint32_t i = 0; i < frames; ++i)
        {
            out1[i] = out2[i] = 0.0f;
        }
    }

    // -------------------------------------------------------------------


    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobSamplerPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobSamplerPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
