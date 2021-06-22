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

#include "OneKnobUI.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobMaximizerUI : public OneKnobUI
{
public:
    OneKnobMaximizerUI();

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char* key, const char* value) override;

private:

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobMaximizerUI)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
