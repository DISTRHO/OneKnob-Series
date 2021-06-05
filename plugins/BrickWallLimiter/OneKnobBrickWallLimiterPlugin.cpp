/*
 * DISTRHO OneKnob BrickWall Limiter
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

#include "OneKnobBrickWallLimiterPlugin.hpp"

START_NAMESPACE_DISTRHO

inline constexpr float db2linear(const float db)
{
    return std::pow(10.0f, 0.05f * db);
}

inline constexpr float logscale50db(const float v)
{
    return (1000.0f / std::exp(6.915f)) * std::exp(0.006915f * v);
}

inline constexpr float eighthoflog(const float v)
{
    return (v + logscale50db(v) * 15) * 0.0625f;
}

inline constexpr float invgain(const float threshold)
{
    return threshold < 0.003f ? 30.0f : eighthoflog(std::max(0.001f, 1.0f / threshold));
}

// -----------------------------------------------------------------------

OneKnobBrickWallLimiterPlugin::OneKnobBrickWallLimiterPlugin()
    : Plugin(kParameterCount, kProgramCount, kStateCount)
{
    // load default values
    loadProgram(kProgramDefault);
}

// -----------------------------------------------------------------------
// Init

void OneKnobBrickWallLimiterPlugin::initParameter(uint32_t index, Parameter& parameter)
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

    case kParameterLineUpdateTickL:
        parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
        parameter.name       = "Tick Left";
        parameter.symbol     = "tick_l";
        parameter.unit       = "";
        parameter.ranges.def = kParameterDefaults[kParameterLineUpdateTickL];
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;

    case kParameterLineUpdateTickR:
        parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
        parameter.name       = "Tick Right";
        parameter.symbol     = "tick_r";
        parameter.unit       = "";
        parameter.ranges.def = kParameterDefaults[kParameterLineUpdateTickR];
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}

void OneKnobBrickWallLimiterPlugin::initProgramName(uint32_t index, String& programName)
{
    switch (index)
    {
    case kProgramDefault:
        programName = "Default";
        break;
    case kProgramGentle:
        programName = "Gentle";
        break;
    case kProgramDestructive:
        programName = "Destructive";
        break;
    case kProgramInsane:
        programName = "Insane";
        break;
    }
}

void OneKnobBrickWallLimiterPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    stateKey = kStateNames[index];
    defaultStateValue = kStateDefaults[index];
}

// -----------------------------------------------------------------------
// Internal data

float OneKnobBrickWallLimiterPlugin::getParameterValue(uint32_t index) const
{
    return parameters[index];
}

void OneKnobBrickWallLimiterPlugin::setParameterValue(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterThreshold:
        threshold_linear = db2linear(value);
        // fall-through
    case kParameterAutoGain:
        parameters[kParameterThreshold] = value;
        break;
    }
}

void OneKnobBrickWallLimiterPlugin::loadProgram(const uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        parameters[kParameterThreshold] = kParameterDefaults[kParameterThreshold];
        parameters[kParameterAutoGain] = 1.0f;
        break;
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
    }

    threshold_linear = db2linear(parameters[kParameterThreshold]);
}

void OneKnobBrickWallLimiterPlugin::setState(const char*, const char*)
{
    // our states are purely UI related, so we do nothing with them on DSP side
}

// -----------------------------------------------------------------------
// Process

void OneKnobBrickWallLimiterPlugin::activate()
{
    // TODO force smoothing into real
}

void OneKnobBrickWallLimiterPlugin::run(const float** const inputs, float** const outputs, const uint32_t frames)
{
    const float* in1  = inputs[0];
    const float* in2  = inputs[1];
    float*       out1 = outputs[0];
    float*       out2 = outputs[1];

    // TODO parameter smoothing
    float threshold = threshold_linear;
    float gain = parameters[kParameterAutoGain] > 0.5f ? invgain(threshold) : 1.0f;

    if (d_isNotEqual(threshold_linear, 1.0f))
    {
        const float threshold_with_gain = threshold_linear * gain;
        float tmp;

        for (uint32_t i=0; i<frames; ++i)
        {
            tmp = *in1++;
            if (tmp > threshold)
                *out1++ = threshold_with_gain;
            else if (tmp < -threshold)
                *out1++ = -threshold_with_gain;
            else
                *out1++ = tmp * gain;

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
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobBrickWallLimiterPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
