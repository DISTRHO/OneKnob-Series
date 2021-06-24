/*
 * DISTRHO OneKnob Filter
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

#pragma once

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobFilterPlugin : public Plugin
{
public:
    OneKnobFilterPlugin();
    ~OneKnobFilterPlugin() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "OneKnob Filter";
    }

    const char* getDescription() const override
    {
        // TODO
        return "";
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
        // TODO
        return "GPLv2+";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'f', 'l');
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

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobFilterPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
