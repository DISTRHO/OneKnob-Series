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

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

OneKnobCompressorPlugin::OneKnobCompressorPlugin()
    : Plugin(kParameterCount, kProgramCount, kStateCount)
{
    // set default values
    loadProgram(kProgramDefault);

    // reset
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
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        // TODO proper range
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
          values[1].label = "Low";
          values[1].value = 1.0f;
          values[2].label = "Mid";
          values[2].value = 2.0f;
          values[3].label = "High";
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

void OneKnobCompressorPlugin::setParameterValue(uint32_t index, float value)
{
    // TODO
    parameters[index] = value;
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

    // reset filter values
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
}

void OneKnobCompressorPlugin::deactivate()
{
}

void OneKnobCompressorPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    const float* in1  = inputs[0];
    const float* in2  = inputs[1];
    float*       out1 = outputs[0];
    float*       out2 = outputs[1];
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobCompressorPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
