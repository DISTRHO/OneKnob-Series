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
    Mutex mutex;

    TwoStageThreadedConvolver()
        : fftconvolver::TwoStageFFTConvolver(),
          Thread("TwoStageThreadedConvolver"),
          semBgProcStart(1),
          semBgProcFinished(0)
    {
    }

    ~TwoStageThreadedConvolver() override
    {
        stop();
    }

    void start()
    {
#ifdef THREADED_CONVOLVER
        startThread(true);
#endif
    }

    void stop()
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
    }

    ~OneKnobConvolutionReverbPlugin() override
    {
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
        case kParameterWetGain:
            parameter.hints      = kParameterIsAutomatable;
            parameter.name       = "Wet Gain";
            parameter.symbol     = "wetGain";
            parameter.unit       = "dB";
            parameter.ranges.def = kParameterDefaults[kParameterWetGain];
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 0.0f;
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

    void initState(uint32_t index, State& state) override
    {
        switch (index)
        {
        case kStateFile:
            state.hints = kStateIsFilenamePath;
            state.key = "irfile";
            state.label = "IR File";
           #ifdef __MOD_DEVICES__
            state.fileTypes = "ir";
           #endif
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

    void setState(const char* const key, const char* const value) override
    {
        if (std::strcmp(key, "irfile") == 0)
        {
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 impulseResponseSize;

            float* const newImpulseResponse = drwav_open_file_and_read_pcm_frames_f32(value, &channels, &sampleRate, &impulseResponseSize, nullptr);
            DISTRHO_SAFE_ASSERT_RETURN(newImpulseResponse != nullptr,);

            float* irBufL;
            float* irBufR;
            switch (channels)
            {
            case 1:
                irBufL = newImpulseResponse;
                irBufR = new float[impulseResponseSize];
                std::memcpy(irBufR, irBufL, sizeof(float)*impulseResponseSize);
                break;
            case 2:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i=0, j=0; i<impulseResponseSize; ++i)
                {
                    irBufL[i] = newImpulseResponse[j++];
                    irBufR[i] = newImpulseResponse[j++];
                }
                break;
            case 4:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i=0, j=0; i<impulseResponseSize; ++i, j+=4)
                {
                    irBufL[i] = newImpulseResponse[j+0] + newImpulseResponse[j+2];
                    irBufR[i] = newImpulseResponse[j+1] + newImpulseResponse[j+3];
                }
                break;
            default:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i=0, j=0; i<impulseResponseSize; ++i)
                {
                    irBufL[i] = irBufR[i] = newImpulseResponse[j];
                    j += channels;
                }
                break;
            }

            const size_t headBlockSize = 128;
            const size_t tailBlockSize = 1024;

            convolverL.stop();
            convolverR.stop();

            {
                const MutexLocker cml(convolverL.mutex);
                convolverL.init(headBlockSize, tailBlockSize, irBufL, impulseResponseSize);
            }

            {
                const MutexLocker cml(convolverR.mutex);
                convolverR.init(headBlockSize, tailBlockSize, irBufR, impulseResponseSize);
            }

            convolverL.start();
            convolverR.start();

            if (irBufL != newImpulseResponse)
                delete[] irBufL;
            if (irBufR != irBufL)
                delete[] irBufR;

            drwav_free(newImpulseResponse, nullptr);
            return;
        }

        OneKnobPlugin::setState(key, value);
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

        for (uint32_t i=0; i<frames; ++i)
        {
            if (!std::isfinite(in1[i]))
                __builtin_unreachable();
            if (!std::isfinite(in2[i]))
                __builtin_unreachable();
            if (!std::isfinite(out1[i]))
                __builtin_unreachable();
            if (!std::isfinite(out2[i]))
                __builtin_unreachable();
        }

        // non-smoothed, we do not care yet
        const float gain = std::pow(10.f, 0.05f * parameters[kParameterWetGain]);

        if (convolverL.mutex.tryLock())
        {
            convolverL.process(in1, out1, frames);
            convolverL.mutex.unlock();
        }
        else
        {
            std::memset(out1, 0, sizeof(float)*frames);
        }

        if (convolverR.mutex.tryLock())
        {
            convolverR.process(in2, out2, frames);
            convolverR.mutex.unlock();
        }
        else
        {
            std::memset(out2, 0, sizeof(float)*frames);
        }

        float tmp1 = lineGraphHighest1;
        float tmp2 = lineGraphHighest2;

        for (uint32_t i=0; i<frames; ++i)
        {
            tmp1 = std::max(tmp1, std::abs(in1[i]));
            tmp1 = std::max(tmp1, std::abs(in2[i]));

            out1[i] *= gain;
            out2[i] *= gain;

            tmp2 = std::max(tmp2, std::abs(out1[i]));
            tmp2 = std::max(tmp2, std::abs(out2[i]));

            if (++lineGraphFrameCounter == lineGraphFrameToReset)
            {
                lineGraphFrameCounter = 0;
                setMeters(tmp1, tmp2);
                tmp1 = tmp2 = 0.f;
            }
        }

        lineGraphHighest1 = tmp1;
        lineGraphHighest2 = tmp2;
    }

    // -------------------------------------------------------------------

private:
    TwoStageThreadedConvolver convolverL, convolverR;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobConvolutionReverbPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobConvolutionReverbPlugin();
}

END_NAMESPACE_DISTRHO
