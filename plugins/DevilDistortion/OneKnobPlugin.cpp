/*
 * DISTRHO OneKnob Devil's Distortion
 * Based on Steve Harris Barry's Satan Maximizer
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 * Copyright (C) 2002-2003 <steve@plugin.org.uk>
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

#define MAXIMIZER_BUFFER_SIZE 16
#define MAXIMIZER_BUFFER_MASK 15

#ifdef __clang__
# define MATH_CONSTEXPR
#else
# define MATH_CONSTEXPR constexpr
#endif

inline MATH_CONSTEXPR float db2linear(const float db)
{
    return std::pow(10.0f, 0.05f * db);
}

// -----------------------------------------------------------------------

class OneKnobDevilDistortionPlugin : public OneKnobPlugin
{
public:
    OneKnobDevilDistortionPlugin()
        : OneKnobPlugin(),
          buffer1(new float[MAXIMIZER_BUFFER_SIZE]),
          buffer2(new float[MAXIMIZER_BUFFER_SIZE])
    {
        init();
    }

    ~OneKnobDevilDistortionPlugin() override
    {
        delete[] buffer1;
        delete[] buffer2;
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        return ""
        "A port of Steve Harris' Barry's Satan Maximizer, formely known as Stupid Compressor.\n"
        "Compresses signals with a stupidly short attack and decay, infinite ratio and hard knee.\n"
        "Not really as a compressor, but good harsh (non-musical) distortion";
    }

    const char* getLicense() const noexcept override
    {
        return "GPLv2+";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'd', 'd');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterKneePoint:
            parameter.hints       = kParameterIsAutomable;
            parameter.name        = "Release";
            parameter.symbol      = "release";
            parameter.unit        = "dB";
            parameter.description = ""
            "Controls the knee roll-off point, ie. the point above which the compression kicks in.\n"
            "0 will have no effect, -90 will remove virtually all dynamic range.";
            parameter.ranges.def  = kParameterDefaults[kParameterKneePoint];
            parameter.ranges.min  = -90.0f;
            parameter.ranges.max  = 0.0f;
            break;

        case kParameterDecayTime:
            parameter.hints       = kParameterIsAutomable | kParameterIsInteger;
            parameter.name        = "Decay Time";
            parameter.symbol      = "decay_time";
            parameter.unit        = "samples";
            parameter.description = "Controls the envelope decay time in samples";
            parameter.ranges.def  = kParameterDefaults[kParameterDecayTime];
            parameter.ranges.min  = 5.0f;
            parameter.ranges.max  = 30.0f;
            if (ParameterEnumerationValue* const values = new ParameterEnumerationValue[6])
            {
                parameter.enumValues.count = 6;
                parameter.enumValues.values = values;

                values[0].label = "5 samples";
                values[0].value = 5.0f;
                values[1].label = "10 samples";
                values[1].value = 10.0f;
                values[2].label = "15  samples";
                values[2].value = 15.0f;
                values[3].label = "20 samples";
                values[3].value = 20.0f;
                values[4].label = "25 samples";
                values[4].value = 25.0f;
                values[5].label = "30 samples";
                values[5].value = 30.0f;
            }
            break;

        default:
            OneKnobPlugin::initParameter(index, parameter);
            break;
        }
    }

    // -------------------------------------------------------------------
    // Process

    void activate() override
    {
        std::memset(buffer1, 0, sizeof(float)*MAXIMIZER_BUFFER_SIZE);
        std::memset(buffer2, 0, sizeof(float)*MAXIMIZER_BUFFER_SIZE);
        buffer_pos = 0;
        env = 0.0f;
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float* in1  = inputs[0];
        const float* in2  = inputs[1];
        float*       out1 = outputs[0];
        float*       out2 = outputs[1];

        // fetch values
        uint buffer_pos_run = buffer_pos;
        float env_run = env;

        const float env_time = parameters[kParameterDecayTime];
        const float knee     = db2linear(parameters[kParameterKneePoint]);
        const uint  delay    = static_cast<uint>(env_time * 0.5f + 0.5f);
        const float env_tr   = 1.0f / env_time;

        float env_sc, in_abs;
        float highestIn = 0.0f, highestOut = 0.0f;

        for (uint32_t i=0; i<frames; ++i)
        {
            in_abs = std::max(std::fabs(in1[i]), std::fabs(in2[i]));
            if (in_abs > env_run) {
                env_run = in_abs;
            } else {
                env_run = in_abs * env_tr + env_run * (1.0f - env_tr);
            }
            if (env_run <= knee) {
                env_sc = 1.0f / knee;
            } else {
                env_sc = 1.0f / env_run;
            }
            buffer1[buffer_pos_run] = in1[i];
            buffer2[buffer_pos_run] = in2[i];
            out1[i] = buffer1[(buffer_pos_run - delay) & MAXIMIZER_BUFFER_MASK] * env_sc;
            out2[i] = buffer2[(buffer_pos_run - delay) & MAXIMIZER_BUFFER_MASK] * env_sc;
            buffer_pos_run = (buffer_pos_run + 1) & MAXIMIZER_BUFFER_MASK;

            highestIn = std::max(highestIn, in_abs);
            highestOut = std::max(highestOut, std::abs(out1[i]));
        }

        // store values
        env = env_run;
        buffer_pos = buffer_pos_run;

        setMeters(highestIn, highestOut);
    }

    // -------------------------------------------------------------------

private:
    float* const buffer1;
    float* const buffer2;
    uint buffer_pos = 0;
    float env = 0.0f;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobDevilDistortionPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobDevilDistortionPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
