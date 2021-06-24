/*
 * DISTRHO OneKnob Series
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

#pragma once

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobPlugin : public Plugin
{
public:
    OneKnobPlugin() : Plugin(kParameterCount, kProgramCount, kStateCount)
    {
        std::memset(parameters, 0, sizeof(parameters));
    }

protected:
    void init()
    {
        // load default values
        loadProgram(kOneKnobProgramDefault);

        // reset state if needed
        deactivate();
    }

    void setMeters(const float in, const float out)
    {
        const float frand = (float)rand() / RAND_MAX * 0.0001f;
        parameters[kParameterCount + kOneKnobParameterLineUpdateTickIn] = (output2nd ? in : -in) + frand;
        parameters[kParameterCount + kOneKnobParameterLineUpdateTickOut] = (output2nd ? out : -out) + frand;
        output2nd = !output2nd;
    }

    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return DISTRHO_PLUGIN_NAME;
    }

    const char* getMaker() const noexcept override
    {
        return DISTRHO_PLUGIN_BRAND;
    }

    const char* getHomePage() const override
    {
        return DISTRHO_PLUGIN_URI;
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(const uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterCount + kOneKnobParameterLineUpdateTickIn:
            parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
            parameter.name       = "Tick In";
            parameter.symbol     = "tick_in";
            parameter.unit       = "";
            parameter.ranges.def = kOneKnobBaseParameterDefaults[kOneKnobParameterLineUpdateTickIn];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;

        case kParameterCount + kOneKnobParameterLineUpdateTickOut:
            parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
            parameter.name       = "Tick Out";
            parameter.symbol     = "tick_out";
            parameter.unit       = "";
            parameter.ranges.def = kOneKnobBaseParameterDefaults[kOneKnobParameterLineUpdateTickOut];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;
        }
    }

    void initProgramName(const uint32_t index, String& programName) override
    {
        switch (index)
        {
        case kOneKnobProgramDefault:
            programName = "Default";
            break;
        }
    }

    void initState(const uint32_t index, String& stateKey, String& defaultStateValue) override
    {
        stateKey = kOneKnobBaseStateNames[index - kStateCount];
        defaultStateValue = kOneKnobBaseStateDefaults[index - kStateCount];
    }

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(const uint32_t index) const override
    {
        return parameters[index];
    }

    void setParameterValue(const uint32_t index, const float value) override
    {
        parameters[index] = value;
    }

    void loadProgram(uint32_t index) override
    {
        switch (index)
        {
        case kOneKnobProgramDefault:
            for (uint i=0; i<kParameterCount; ++i)
                parameters[i] = kParameterDefaults[i];
            break;
        }
    }

    void setState(const char*, const char*) override
    {
        // our states are purely UI related, so we do nothing with them on DSP side
    }

    // -------------------------------------------------------------------

    float parameters[kParameterCount + kOneKnobBaseParameterCount];

private:
    bool output2nd = false;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
