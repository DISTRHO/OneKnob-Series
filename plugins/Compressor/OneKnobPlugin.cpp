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

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "OneKnobPlugin.hpp"

#include "compressor_core.c"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobCompressorPlugin : public OneKnobPlugin
{
public:
    OneKnobCompressorPlugin()
        : OneKnobPlugin()
    {
        compressor_init(&compressor, getSampleRate());
        init();
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        // TODO stereo vs mono
        return "A stupid & simple audio compressor with a single knob, part of the DISTRHO OneKnob Series";
    }

    const char* getLicense() const noexcept override
    {
        // TODO
        return "LGPL";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'c', 'o');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterRelease:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Release";
            parameter.symbol     = "release";
            parameter.unit       = "ms";
            parameter.ranges.def = kParameterDefaults[kParameterRelease];
            parameter.ranges.min = 50.0f;
            parameter.ranges.max = 500.0f;
            break;
        case kParameterMode:
            parameter.hints      = kParameterIsAutomable | kParameterIsInteger;
            parameter.name       = "Mode";
            parameter.symbol     = "mode";
            parameter.unit       = "";
            parameter.ranges.def = kParameterDefaults[kParameterMode];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 3.0f;
            if (ParameterEnumerationValue* const values = new ParameterEnumerationValue[4])
            {
              parameter.enumValues.count = 4;
              parameter.enumValues.values = values;
              parameter.enumValues.restrictedMode = true;

              values[0].label = "Off";
              values[0].value = 0.0f;
              values[1].label = "Light";
              values[1].value = 1.0f;
              values[2].label = "Mild";
              values[2].value = 2.0f;
              values[3].label = "Heavy";
              values[3].value = 3.0f;
            }
            break;
        }
    }

    void initProgramName(uint32_t index, String& programName) override
    {
        switch (index)
        {
        case kProgramDefault:
            programName = "Default";
            break;
        case kProgramConservative:
            programName = "Conservative";
            break;
        case kProgramLiberal:
            programName = "Liberal";
            break;
        case kProgramExtreme:
            programName = "Extreme";
            break;
        }
    }

    // -------------------------------------------------------------------
    // Internal data

    void setParameterValue(uint32_t index, float value) override
    {
        OneKnobPlugin::setParameterValue(index, value);

        switch (index)
        {
        case kParameterRelease:
        case kParameterMode:
        {
            const float release = parameters[kParameterRelease];
            const int mode = static_cast<int>(parameters[kParameterMode] + 0.5f);

            switch (mode)
            {
            case 1: // Light
                compressor_set_params(&compressor, -12.f, 12.f, 2.f, 0.0001f, release/1000.f, -3.f);
                break;
            case 2: // Mild
                compressor_set_params(&compressor, -12.f, 12.f, 3.f, 0.0001f, release/1000.f, -3.f);
                break;
            case 3: // Heavy
                compressor_set_params(&compressor, -15.f, 15.f, 4.f, 0.0001f, release/1000.f, -3.f);
                break;
            case 4: // Extreme
                compressor_set_params(&compressor, -25.f, 15.f, 10.f, 0.0001f, release/1000.f, -6.f);
                break;
            }

            compressorOn = mode >= 1 && mode <= 3;
        }
            break;
        }
    }

    void loadProgram(uint32_t index) override
    {
        switch (index)
        {
        case kProgramDefault:
            loadDefaultParameterValues();
            break;
        case kProgramConservative:
            parameters[kParameterRelease] = 100.0f;
            parameters[kParameterMode] = 1.0f;
            break;
        case kProgramLiberal:
            parameters[kParameterRelease] = 100.0f;
            parameters[kParameterMode] = 2.0f;
            break;
        case kProgramExtreme:
            parameters[kParameterRelease] = 100.0f;
            parameters[kParameterMode] = 3.0f;
            break;
        }

        // activate filter parameters
        activate();
    }

    // -------------------------------------------------------------------
    // Process

    void activate() override
    {
        OneKnobPlugin::activate();

        setParameterValue(kParameterRelease, parameters[kParameterRelease]);
    }

    void deactivate() override
    {
        compressor_init(&compressor, getSampleRate());
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* in1  = inputs[0];
        const float* in2  = inputs[1];
        float*       out1 = outputs[0];
        float*       out2 = outputs[1];

        if (compressorOn)
        {
            compressor_process(&compressor, frames, in1, in2, out1, out2);
        }
        else
        {
            if (out1 != in1)
                std::memcpy(out1, in1, sizeof(float)*frames);
            if (out2 != in2)
                std::memcpy(out2, in2, sizeof(float)*frames);
        }

        float tmp;
        for (uint32_t i=0; i<frames; ++i)
        {
            tmp = *in1++;
            lineGraphHighest1 = std::max(lineGraphHighest1, std::abs(tmp));
            tmp = *out1++;
            lineGraphHighest2 = std::max(lineGraphHighest2, std::abs(tmp));

            if (++lineGraphFrameCounter == lineGraphFrameToReset)
            {
                lineGraphFrameCounter = 0;
                setMeters(lineGraphHighest1, lineGraphHighest2);
                lineGraphHighest1 = lineGraphHighest2 = 0.0f;
            }
        }
    }


    // -------------------------------------------------------------------

private:
    sf_compressor_state_st compressor;
    bool compressorOn = false;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobCompressorPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobCompressorPlugin();
}

END_NAMESPACE_DISTRHO
