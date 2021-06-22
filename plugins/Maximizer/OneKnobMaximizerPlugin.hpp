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

#pragma once

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "DistrhoPlugin.hpp"

#define MAXIMIZER_BUFFER_SIZE 16
#define MAXIMIZER_BUFFER_MASK 15

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobMaximizerPlugin : public Plugin
{
public:
    OneKnobMaximizerPlugin();
    ~OneKnobMaximizerPlugin() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "OneKnob Maximizer";
    }

    const char* getDescription() const override
    {
        return ""
        "A port of Steve Harris' Barry's Satan Maximizer, formely known as Stupid Compressor.\n"
        "Compresses signals with a stupidly short attack and decay, infinite ratio and hard knee.\n"
        "Not really as a compressor, but good harsh (non-musical) distortion";
    }

    const char* getMaker() const noexcept override
    {
        return "DISTRHO";
    }

    const char* getHomePage() const override
    {
        return DISTRHO_PLUGIN_URI;
    }

    const char* getLicense() const noexcept override
    {
        return "GPLv2+";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'm', 'x');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;
    void initState(uint32_t index, String& stateKey, String& defaultStateValue) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    void loadProgram(uint32_t index) override;
    void setState(const char* key, const char* value) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:
    float parameters[kParameterCount];
    float* const buffer1;
    float* const buffer2;
    uint buffer_pos = 0;
    float env = 0.0f;
    bool output2nd = false;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobMaximizerPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
