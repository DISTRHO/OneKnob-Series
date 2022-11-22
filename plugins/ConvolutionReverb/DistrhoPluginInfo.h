/*
 * DISTRHO OneKnob Convolution Reverb
 * Copyright (C) 2022 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include "OneKnobPluginInfo.h"

#define DISTRHO_PLUGIN_NAME    "OneKnob Convolution Reverb"
#define DISTRHO_PLUGIN_URI     "https://distrho.kx.studio/plugins/oneknob#ConvolutionReverb"
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.oneknob.ConvolutionReverb"

#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "reverb", "stereo"
#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:ReverbPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Reverb|Stereo"

#undef DISTRHO_PLUGIN_IS_RT_SAFE
#define DISTRHO_PLUGIN_IS_RT_SAFE 0

enum Parameters {
    kParameterDryLevel,
    kParameterWetLevel,
    kParameterLowPassFilter,
    kParameterCount
};

enum Programs {
    kProgramDefault,
    kProgramCount
};

enum States {
    kStateFile,
    kStateCount
};

static const float kParameterDefaults[kParameterCount] = {
    -60.f,
    -30.f,
    0.f
};
