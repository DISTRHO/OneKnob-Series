/*
 * DISTRHO OneKnob Filter
 * Copyright (C) 2023 Filipe Coelho <falktx@falktx.com>
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
#include "Biquad.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobFilterPlugin : public OneKnobPlugin
{
public:
    OneKnobFilterPlugin()
        : OneKnobPlugin(),
          filter(bq_type_lowpass,
                 kParameterRanges[kParameterFrequency].def / getSampleRate(),
                 kParameterRanges[kParameterQ].def,
                 kParameterRanges[kParameterGain].def)
       #if DISTRHO_PLUGIN_NUM_INPUTS == 2
        , filter2(bq_type_lowpass,
                  kParameterRanges[kParameterFrequency].def / getSampleRate(),
                  kParameterRanges[kParameterQ].def,
                  kParameterRanges[kParameterGain].def)
       #endif
    {
        init();
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        return ""
        "One-Knob Filter is a multi-type filter used for plugin design.\n"
        "\n"

        "Type: Selects the type of filter.\n"
        "Frequency: Sets the center frequency for emphasis or attenuation.\n"
        "Q/Order: Adjusts the width/order of the affected frequency range. Does NOT have any effect with the Low Shelf and High Shelf types.\n"
        "Gain:  Controls the overall amplitude or level of the filtered frequencies. Does NOT have any effect with the Low Pass and High Pass filters.";
    }

    const char* getLicense() const noexcept override
    {
        return "GPLv2+";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'f', 't');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterType:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger;
            parameter.name   = "Type";
            parameter.symbol = "type";
            parameter.ranges.def = kParameterRanges[kParameterType].def;
            parameter.ranges.min = kParameterRanges[kParameterType].min;
            parameter.ranges.max = kParameterRanges[kParameterType].max;
            if (ParameterEnumerationValue* const values = new ParameterEnumerationValue[7])
            {
                parameter.enumValues.count = 7;
                parameter.enumValues.values = values;
                parameter.enumValues.restrictedMode = true;

                values[0].label = "Lowpass";
                values[0].value = bq_type_lowpass;
                values[1].label = "Highpass";
                values[1].value = bq_type_highpass;
                values[2].label = "Bandpass";
                values[2].value = bq_type_bandpass;
                values[3].label = "Notch";
                values[3].value = bq_type_notch;
                values[4].label = "Peak";
                values[4].value = bq_type_peak;
                values[5].label = "Lowshelf";
                values[5].value = bq_type_lowshelf;
                values[6].label = "Highshelf";
                values[6].value = bq_type_highshelf;
            }
            break;
        case kParameterFrequency:
            parameter.hints      = kParameterIsAutomatable | kParameterIsLogarithmic;
            parameter.name       = "Frequency";
            parameter.symbol     = "frequency";
            parameter.unit       = "Hz";
            parameter.ranges.def = kParameterRanges[kParameterFrequency].def;
            parameter.ranges.min = kParameterRanges[kParameterFrequency].min;
            parameter.ranges.max = kParameterRanges[kParameterFrequency].max;
            break;
        case kParameterQ:
            parameter.hints      = kParameterIsAutomatable;
            parameter.name       = "Q";
            parameter.symbol     = "q";
            parameter.unit       = "";
            parameter.ranges.def = kParameterRanges[kParameterQ].def;
            parameter.ranges.min = kParameterRanges[kParameterQ].min;
            parameter.ranges.max = kParameterRanges[kParameterQ].max;
            break;
        case kParameterGain:
            parameter.hints      = kParameterIsAutomatable;
            parameter.name       = "Gain";
            parameter.symbol     = "Gain";
            parameter.unit       = "dB";
            parameter.ranges.def = kParameterRanges[kParameterGain].def;
            parameter.ranges.min = kParameterRanges[kParameterGain].min;
            parameter.ranges.max = kParameterRanges[kParameterGain].max;
            break;
        case kParameterBypass:
            parameter.initDesignation(kParameterDesignationBypass);
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
        }
    }

    // -------------------------------------------------------------------
    // Internal data

    void setParameterValue(uint32_t index, float value) override
    {
        switch (index)
        {
        case kParameterType:
            filter.setType(static_cast<int>(value + 0.5f));
           #if DISTRHO_PLUGIN_NUM_INPUTS == 2
            filter2.setType(static_cast<int>(value + 0.5f));
           #endif
            break;
        case kParameterFrequency:
            filter.setFc(value / getSampleRate());
           #if DISTRHO_PLUGIN_NUM_INPUTS == 2
            filter2.setFc(value / getSampleRate());
           #endif
            break;
        case kParameterQ:
            filter.setQ(value);
           #if DISTRHO_PLUGIN_NUM_INPUTS == 2
            filter2.setQ(value);
           #endif
            break;
        case kParameterGain:
            filter.setPeakGain(value);
           #if DISTRHO_PLUGIN_NUM_INPUTS == 2
            filter2.setPeakGain(value);
           #endif
            break;
        }

        OneKnobPlugin::setParameterValue(index, value);
    }

    void loadProgram(uint32_t index) override
    {
        switch (index)
        {
        case kProgramDefault:
            loadDefaultParameterValues();
            break;
        }

        // activate filter parameters
        activate();
    }

    // -------------------------------------------------------------------
    // Process

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* in = inputs[0];
        float* out = outputs[0];

        const bool bypassed = parameters[kParameterBypass] > 0.5f;

       #if DISTRHO_PLUGIN_NUM_INPUTS == 1
        if (bypassed)
        {
            if (out != in)
                std::memcpy(out, in, sizeof(float)*frames);
            return;
        }

        for (uint32_t i = 0; i < frames; ++i)
        {
            if (!std::isfinite(in[i]))
                __builtin_unreachable();

            out[i] = filter.process(in[i]);

            if (!std::isfinite(out[i]))
                __builtin_unreachable();
        }
       #else
        const float* in2 = inputs[1];
        float* out2 = outputs[1];

        if (bypassed)
        {
            if (out != in)
                std::memcpy(out, in, sizeof(float)*frames);
            if (out2 != in2)
                std::memcpy(out2, in2, sizeof(float)*frames);
            return;
        }

        for (uint32_t i = 0; i < frames; ++i)
        {
            if (!std::isfinite(in[i]))
                __builtin_unreachable();
            if (!std::isfinite(in2[i]))
                __builtin_unreachable();

            out[i] = filter.process(in[i]);
            out2[i] = filter.process(in2[i]);

            if (!std::isfinite(out[i]))
                __builtin_unreachable();
            if (!std::isfinite(out2[i]))
                __builtin_unreachable();
        }
       #endif
    }

    // -------------------------------------------------------------------

private:
    Biquad filter;
   #if DISTRHO_PLUGIN_NUM_INPUTS == 2
    Biquad filter2;
   #endif

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobFilterPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobFilterPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
