/*
 * DISTRHO OneKnob Maximizer
 * Based on Steve Harris Barry's Satan Maximizer
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 * Copyright (C) 2002-2003 <steve@plugin.org.uk>
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

#include "OneKnobMaximizerPlugin.hpp"

START_NAMESPACE_DISTRHO

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

OneKnobMaximizerPlugin::OneKnobMaximizerPlugin()
    : Plugin(kParameterCount, kProgramCount, kStateCount),
      buffer1(new float[MAXIMIZER_BUFFER_SIZE]),
      buffer2(new float[MAXIMIZER_BUFFER_SIZE])
{
    // load default values
    loadProgram(kProgramDefault);
}

OneKnobMaximizerPlugin::~OneKnobMaximizerPlugin()
{
    delete[] buffer1;
    delete[] buffer2;
}

// -----------------------------------------------------------------------
// Init

void OneKnobMaximizerPlugin::initParameter(uint32_t index, Parameter& parameter)
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

    case kParameterLineUpdateTickIn:
        parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
        parameter.name       = "Tick In";
        parameter.symbol     = "tick_in";
        parameter.unit       = "";
        parameter.ranges.def = kParameterDefaults[kParameterLineUpdateTickIn];
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;

    case kParameterLineUpdateTickOut:
        parameter.hints      = kParameterIsAutomable | kParameterIsOutput;
        parameter.name       = "Tick Out";
        parameter.symbol     = "tick_out";
        parameter.unit       = "";
        parameter.ranges.def = kParameterDefaults[kParameterLineUpdateTickOut];
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}

void OneKnobMaximizerPlugin::initProgramName(uint32_t index, String& programName)
{
    switch (index)
    {
    case kProgramDefault:
        programName = "Default";
        break;
    }
}

void OneKnobMaximizerPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    stateKey = kStateNames[index];
    defaultStateValue = kStateDefaults[index];
}

// -----------------------------------------------------------------------
// Internal data

float OneKnobMaximizerPlugin::getParameterValue(uint32_t index) const
{
    return parameters[index];
}

void OneKnobMaximizerPlugin::setParameterValue(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterKneePoint:
    case kParameterDecayTime:
        parameters[index] = value;
        break;
    }
}

void OneKnobMaximizerPlugin::loadProgram(uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        parameters[kParameterKneePoint] = kParameterDefaults[kParameterKneePoint];
        parameters[kParameterDecayTime] = kParameterDefaults[kParameterDecayTime];
        break;
    }
}

void OneKnobMaximizerPlugin::setState(const char*, const char*)
{
    // our states are purely UI related, so we do nothing with them on DSP side
}

// -----------------------------------------------------------------------
// Process

void OneKnobMaximizerPlugin::activate()
{
    std::memset(buffer1, 0, sizeof(float)*MAXIMIZER_BUFFER_SIZE);
    std::memset(buffer2, 0, sizeof(float)*MAXIMIZER_BUFFER_SIZE);
    buffer_pos = 0;
    env = 0.0f;
}

void OneKnobMaximizerPlugin::run(const float** const inputs, float** const outputs, const uint32_t frames)
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

    parameters[kParameterLineUpdateTickIn] = (output2nd ? highestIn : -highestIn) + (float)rand() / RAND_MAX * 0.0001;
    parameters[kParameterLineUpdateTickOut] = (output2nd ? highestOut : -highestOut) + (float)rand() / RAND_MAX * 0.0001;
    output2nd = !output2nd;
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobMaximizerPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
