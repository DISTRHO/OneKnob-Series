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

#pragma once

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

    void initParameter(uint32_t index, Parameter& parameter) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:
    float* const buffer1;
    float* const buffer2;
    uint buffer_pos = 0;
    float env = 0.0f;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobDevilDistortionPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
