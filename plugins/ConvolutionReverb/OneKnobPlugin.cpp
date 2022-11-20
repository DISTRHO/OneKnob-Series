/*
 * DISTRHO OneKnob Convolution Reverb
 * Copyright (C) 2022 Filipe Coelho <falktx@falktx.com>
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

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "OneKnobPlugin.hpp"
#include "extra/Thread.hpp"
#include "Semaphore.hpp"

#include "dr_wav.h"
#include "FFTConvolver/TwoStageFFTConvolver.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

#define THREADED_CONVOLVER

class TwoStageThreadedConvolver : public fftconvolver::TwoStageFFTConvolver,
                                  private Thread
{
    Semaphore semBgProcStart;
    Semaphore semBgProcFinished;

public:
    TwoStageThreadedConvolver()
        : fftconvolver::TwoStageFFTConvolver(),
          Thread("TwoStageThreadedConvolver"),
          semBgProcStart(1),
          semBgProcFinished(0)
    {
#ifdef THREADED_CONVOLVER
        startThread(true);
#endif
    }

    ~TwoStageThreadedConvolver() override
    {
#ifdef THREADED_CONVOLVER
        signalThreadShouldExit();
        semBgProcStart.post();
        stopThread(5000);
#endif
    }

protected:
#ifdef THREADED_CONVOLVER
    void startBackgroundProcessing() override
    {
        semBgProcStart.post();
    }

    void waitForBackgroundProcessing() override
    {
        if (isThreadRunning() && !shouldThreadExit())
            semBgProcFinished.wait();
    }
#endif

    void run() override
    {
#ifdef THREADED_CONVOLVER
        while (!shouldThreadExit())
        {
            semBgProcStart.wait();

            if (shouldThreadExit())
                break;

            doBackgroundProcessing();
            semBgProcFinished.post();
        }
#endif
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoStageThreadedConvolver)
};

// -----------------------------------------------------------------------

class OneKnobConvolutionReverbPlugin : public OneKnobPlugin
{
public:
    OneKnobConvolutionReverbPlugin()
        : OneKnobPlugin()
    {
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 impulseResponseSize;

        if (impulseResponse == nullptr)
        {
            impulseResponse = drwav_open_file_and_read_pcm_frames_f32("/home/falktx/Source/DISTRHO/OneKnob-Series/plugins/ConvolutionReverb/hall1-medium.wav", &channels, &sampleRate, &impulseResponseSize, nullptr);
            DISTRHO_SAFE_ASSERT(impulseResponse != nullptr);
        }

        const size_t headBlockSize = 64;
        const size_t tailBlockSize = 1024;
        convolverL.init(headBlockSize, tailBlockSize, impulseResponse, impulseResponseSize);
        convolverR.init(headBlockSize, tailBlockSize, impulseResponse, impulseResponseSize);
    }

    ~OneKnobConvolutionReverbPlugin() override
    {
        drwav_free(impulseResponse, nullptr);
    }

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getDescription() const override
    {
        // TODO stereo vs mono
        return "";
    }

    const char* getLicense() const noexcept override
    {
        // TODO
        return "";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'c', 'r');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterWet:
            parameter.hints      = kParameterIsAutomatable;
            parameter.name       = "Wet";
            parameter.symbol     = "wet";
            parameter.unit       = "%";
            parameter.ranges.def = kParameterDefaults[kParameterWet];
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 100.0f;
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

    void activate() override
    {
        OneKnobPlugin::activate();
    }

    void deactivate() override
    {
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* in1  = inputs[0];
        const float* in2  = inputs[1];
        float*       out1 = outputs[0];
        float*       out2 = outputs[1];

        convolverL.process(in1, out1, frames);
        convolverR.process(in2, out2, frames);
    }

    // -------------------------------------------------------------------

private:
    TwoStageThreadedConvolver convolverL, convolverR;
    float* impulseResponse = nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobConvolutionReverbPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobConvolutionReverbPlugin();
}

END_NAMESPACE_DISTRHO
