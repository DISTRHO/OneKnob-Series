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
        parameter.ranges.def = kParameterDefaultRelease;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        // TODO proper range
        break;

    case kParameterMode:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Mode";
        parameter.symbol     = "mode";
        parameter.unit       = "";
        parameter.ranges.def = kParameterDefaultMode;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 3.0f;
        // TODO scalepoints
        break;

    case kParameterLineUpdateTickL:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Tick Left";
        parameter.symbol     = "tick_l";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        // TODO output
        break;

    case kParameterLineUpdateTickL:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Tick Right";
        parameter.symbol     = "tick_r";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        // TODO output
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
        parameters[kParameterRelease] = kParameterDefaultRelease;
        parameters[kParameterMode] = kParameterDefaultMode;
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
