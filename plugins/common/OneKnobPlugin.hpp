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
#include "SharedMemory.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobPlugin : public Plugin
{
public:
    OneKnobPlugin()
        : Plugin(kParameterCount, kProgramCount, 0),
          lineGraphFrameToReset(getSampleRate() / 120)
    {
        std::memset(parameters, 0, sizeof(parameters));
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
    // Parameters

    float getParameterValue(const uint32_t index) const override
    {
        return parameters[index];
    }

    void setParameterValue(const uint32_t index, const float value) override
    {
        parameters[index] = value;
    }

    // -------------------------------------------------------------------
    // State

    void setState(const char* const key, const char* const value) override
    {
        if (std::strcmp(key, "filemapping") == 0)
        {
            if (lineGraphsData.isCreatedOrConnected())
            {
                DISTRHO_SAFE_ASSERT(! lineGraphActive);
                lineGraph1.setFloatFifo(nullptr);
                lineGraph2.setFloatFifo(nullptr);
                lineGraphsData.close();
            }

            if (OneKnobLineGraphFifos* const fifos = lineGraphsData.connect(value))
            {
                lineGraph1.setFloatFifo(&fifos->v1);
                lineGraph2.setFloatFifo(&fifos->v2);
                lineGraphActive = true;
            }
        }
    }

    // -------------------------------------------------------------------
    // Process

    void activate() override
    {
        lineGraphFrameCounter = 0;
        lineGraphHighest1 = lineGraphHighest2 = 0.0f;
    }

    void sampleRateChanged(const double newSampleRate) override
    {
        lineGraphFrameToReset = newSampleRate / 120;
    }

    // -------------------------------------------------------------------

    void init()
    {
        // load values of default/first program
        loadDefaultParameterValues();

        // reset state if needed
        deactivate();
    }

    void loadDefaultParameterValues()
    {
        for (uint i=0; i<kParameterCount; ++i)
            parameters[i] = kParameterDefaults[i];
    }

    inline void setMeters(const float v1, const float v2)
    {
        if (! lineGraphActive)
            return;

        if (lineGraphsData.getDataPointer()->closed)
        {
            lineGraphActive = false;
            return;
        }

        lineGraph1.write(v1);
        lineGraph2.write(v2);
    }

    // -------------------------------------------------------------------

    float parameters[kParameterCount];

    bool lineGraphActive = false;
    uint32_t lineGraphFrameCounter = 0;
    uint32_t lineGraphFrameToReset;
    float lineGraphHighest1 = 0.0f;
    float lineGraphHighest2 = 0.0f;

private:
    OneKnobFloatFifoControl lineGraph1;
    OneKnobFloatFifoControl lineGraph2;
    SharedMemory<OneKnobLineGraphFifos> lineGraphsData;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
