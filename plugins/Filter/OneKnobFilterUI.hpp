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

#include "OneKnobUI.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobFilterUI : public OneKnobUI
{
public:
    OneKnobFilterUI();

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char* key, const char* value) override;

private:

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobFilterUI)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
