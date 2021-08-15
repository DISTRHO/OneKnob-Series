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

/* evil bastard */
#define private public
#include "FloatFifo.hpp"
#undef private

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

#ifdef __clang__
# define MATH_CONSTEXPR
#else
# define MATH_CONSTEXPR constexpr
#endif

// -----------------------------------------------------------------------

class OneKnobPlugin : public Plugin
{
    static const uint32_t kMaxLineGraphSamples = 512;

public:
    OneKnobPlugin() : Plugin(kParameterCount + kOneKnobBaseParameterCount,
                             kProgramCount + kOneKnobBaseProgramCount, 
                             kStateCount + kOneKnobBaseStateCount)
    {
        std::memset(parameters, 0, sizeof(parameters));

        // real numSamples depends on buffer size
        lineGraphFifoIn.alloc(kMaxLineGraphSamples);
        lineGraphFifoOut.alloc(kMaxLineGraphSamples);
        bufferSizeChanged(getBufferSize());
    }

protected:
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

    void initParameter(uint32_t, Parameter&) override
    {
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

    void bufferSizeChanged(const uint32_t newBufferSize) override
    {
        const uint32_t numSamples = std::min(kMaxLineGraphSamples, newBufferSize/32);
        lineGraphFifoIn.fifo.numSamples = lineGraphFifoOut.fifo.numSamples = numSamples;
        lineGraphFifoIn.clearData();
        lineGraphFifoOut.clearData();
    }

    // -------------------------------------------------------------------

    void init()
    {
        // load default values
        loadProgram(kOneKnobProgramDefault);

        // reset state if needed
        deactivate();
    }

    void setMeters(const float in, const float out)
    {
        lineGraphFifoIn.write(in);
        lineGraphFifoOut.write(out);
    }

    // -------------------------------------------------------------------

    float parameters[kParameterCount + kOneKnobBaseParameterCount];

public: // TODO setup shared memory
    HeapFloatFifo lineGraphFifoIn, lineGraphFifoOut;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
