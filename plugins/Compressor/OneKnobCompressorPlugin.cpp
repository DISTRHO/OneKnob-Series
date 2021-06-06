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

#include "OneKnobCompressorPlugin.hpp"

#include "compressor_core.c"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

OneKnobCompressorPlugin::OneKnobCompressorPlugin()
    : Plugin(kParameterCount, kProgramCount, kStateCount)
{
    // load default values
    loadProgram(kProgramDefault);

    // reset filter
    deactivate();
}

// -----------------------------------------------------------------------
// Init

void OneKnobCompressorPlugin::initParameter(uint32_t index, Parameter& parameter)
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
        parameter.ranges.max = 1000.0f;
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

void OneKnobCompressorPlugin::initProgramName(uint32_t index, String& programName)
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

void OneKnobCompressorPlugin::initState(uint32_t index, String& stateKey, String& defaultStateValue)
{
    stateKey = kStateNames[index];
    defaultStateValue = kStateDefaults[index];
}

// -----------------------------------------------------------------------
// Internal data

float OneKnobCompressorPlugin::getParameterValue(uint32_t index) const
{
    return parameters[index];
}

void OneKnobCompressorPlugin::setParameterValue(const uint32_t index, const float value)
{
    switch (index)
    {
    case kParameterRelease:
    case kParameterMode:
    {
        parameters[index] = value;

        const float release = parameters[kParameterRelease];
        const int mode = static_cast<int>(parameters[kParameterMode] + 0.5f);

        switch (mode)
        {
        case 1: // Light
            compressor_set_params(&compressor, -12.f, 12.f, 2.f, 0.01f, release/1000.f, -3.f);
            break;
        case 2: // Mild
            compressor_set_params(&compressor, -12.f, 12.f, 3.f, 0.01f, release/1000.f, -3.f);
            break;
        case 3: // Heavy
            compressor_set_params(&compressor, -15.f, 15.f, 4.f, 0.01f, release/1000.f, -3.f);
            /*
            compressor_set_params(&compressor, -25.f, 15.f, 10.f, 0.0001f, release/1000.f, -6.f);
            */
            break;
        }

        compressorOn = mode >= 1 && mode <= 3;
        d_stdout("setParameterValue %i %f %i", compressorOn, release, mode);
    }
        break;
    }
}

void OneKnobCompressorPlugin::loadProgram(uint32_t index)
{
    switch (index)
    {
    case kProgramDefault:
        parameters[kParameterRelease] = kParameterDefaults[kParameterRelease];
        parameters[kParameterMode] = kParameterDefaults[kParameterMode];
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

void OneKnobCompressorPlugin::setState(const char*, const char*)
{
    // our states are purely UI related, so we do nothing with them on DSP side
}

// -----------------------------------------------------------------------
// Process

void OneKnobCompressorPlugin::activate()
{
    setParameterValue(kParameterRelease, parameters[kParameterRelease]);
}

void OneKnobCompressorPlugin::deactivate()
{
    compressor_init(&compressor, getSampleRate());
}

void OneKnobCompressorPlugin::run(const float** const inputs, float** const outputs, const uint32_t frames)
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

    float tmp, highest = 0.0f;
    for (uint32_t i=0; i<frames; ++i)
    {
        tmp = *in1++;
        highest = std::max(highest, std::abs(tmp));
    }

    parameters[kParameterLineUpdateTickL] = highest;
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobCompressorPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
