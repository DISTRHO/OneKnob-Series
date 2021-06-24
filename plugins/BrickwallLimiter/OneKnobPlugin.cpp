/*
 * DISTRHO OneKnob Brickwall Limiter
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

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

#ifdef __clang__
# define MATH_CONSTEXPR
#else
# define MATH_CONSTEXPR constexpr
#endif

inline MATH_CONSTEXPR float db2linear(const float db)
{
    return std::pow(10.0f, 0.05f * db);
}

inline MATH_CONSTEXPR float logscale50db(const float v)
{
    return (1000.0f / std::exp(6.915f)) * std::exp(0.006915f * v);
}

inline MATH_CONSTEXPR float sixteenthoflog(const float v)
{
    return (v + logscale50db(v) * 15) * 0.0625f;
}

inline MATH_CONSTEXPR float invgain(const float linearThreshold)
{
    return linearThreshold < 0.003f ? 30.0f : sixteenthoflog(std::max(0.001f, 1.0f / linearThreshold));
}

// -----------------------------------------------------------------------

class OneKnobBrickwallLimiterPlugin : public OneKnobPlugin
{
public:
    OneKnobBrickwallLimiterPlugin()
        : OneKnobPlugin()
    {
        init();
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        // TODO stereo vs mono
        return "A stupid & simple brick-wall style limiter, part of the DISTRHO OneKnob Series";
    }

    const char* getLicense() const noexcept override
    {
        // TODO
        return "LGPL";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'b', 'w');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(const uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterThreshold:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Threshold";
            parameter.symbol     = "threshold";
            parameter.unit       = "dB";
            parameter.ranges.def = kParameterDefaults[kParameterThreshold];
            parameter.ranges.min = -50.0f;
            parameter.ranges.max = 0.0f;
            break;

        case kParameterAutoGain:
            parameter.hints      = kParameterIsAutomable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name       = "Auto-Gain";
            parameter.symbol     = "autogain";
            parameter.unit       = "";
            parameter.ranges.def = kParameterDefaults[kParameterAutoGain];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;

        default:
            OneKnobPlugin::initParameter(index, parameter);
            break;
        }
    }

    void initProgramName(uint32_t index, String& programName) override
    {
        switch (index)
        {
        case kProgramGentle:
            programName = "Gentle";
            break;
        case kProgramDestructive:
            programName = "Destructive";
            break;
        case kProgramInsane:
            programName = "Insane";
            break;
        default:
            OneKnobPlugin::initProgramName(index, programName);
            break;
        }
    }

    // -------------------------------------------------------------------
    // Internal data

    void setParameterValue(const uint32_t index, const float value) override
    {
        OneKnobPlugin::setParameterValue(index, value);

        if (index == kParameterThreshold)
            threshold_linear = db2linear(value);
    }

    void loadProgram(const uint32_t index) override
    {
        switch (index)
        {
        case kProgramGentle:
            parameters[kParameterThreshold] = -8.0f;
            parameters[kParameterAutoGain] = 1.0f;
            break;
        case kProgramDestructive:
            parameters[kParameterThreshold] = -20.0f;
            parameters[kParameterAutoGain] = 1.0f;
            break;
        case kProgramInsane:
            parameters[kParameterThreshold] = -36.0f;
            parameters[kParameterAutoGain] = 1.0f;
            break;
        default:
            OneKnobPlugin::loadProgram(index);
            break;
        }

        threshold_linear = db2linear(parameters[kParameterThreshold]);
    }

    // -------------------------------------------------------------------
    // Process

    void activate() override
    {
        // TODO force smoothing into real
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float* in1  = inputs[0];
        const float* in2  = inputs[1];
        float*       out1 = outputs[0];
        float*       out2 = outputs[1];

        // TODO parameter smoothing
        const float threshold = threshold_linear;
        const float gain = parameters[kParameterAutoGain] > 0.5f ? invgain(threshold) : 1.0f;
        float tmp;

        // TESTING
        float highestIn = 0.0f, highestOut = 0.0f;

        if (d_isNotEqual(threshold, 1.0f))
        {
            const float threshold_with_gain = threshold * gain;

            for (uint32_t i=0; i<frames; ++i)
            {
                tmp = *in1++;
                highestIn = std::max(highestIn, std::abs(tmp));

                if (tmp > threshold)
                    tmp = threshold_with_gain;
                else if (tmp < -threshold)
                    tmp = -threshold_with_gain;
                else
                    tmp *= gain;

                *out1++ = tmp;
                highestOut = std::max(highestOut, std::abs(tmp));

                tmp = *in2++;
                if (tmp > threshold)
                    *out2++ = threshold_with_gain;
                else if (tmp < -threshold)
                    *out2++ = -threshold_with_gain;
                else
                    *out2++ = tmp * gain;
            }
        }
        else
        {
            if (out1 != in1)
                std::memcpy(out1, in1, sizeof(float)*frames);
            if (out2 != in2)
                std::memcpy(out2, in2, sizeof(float)*frames);

            for (uint32_t i=0; i<frames; ++i)
            {
                tmp = *in1++;
                highestIn = std::max(highestIn, std::abs(tmp));
                tmp = *out1++;
                highestOut = std::max(highestOut, std::abs(tmp));
            }
        }

        setMeters(highestIn, highestOut);
    }

    // -------------------------------------------------------------------

private:
    float threshold_linear;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobBrickwallLimiterPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobBrickwallLimiterPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
