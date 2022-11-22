/*
 * DISTRHO OneKnob Convolution Reverb
 * Copyright (C) 2022 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

// IDE helper (not needed for building)
#include "DistrhoPluginInfo.h"

#include "LinearSmoother.hpp"
#include "OneKnobPlugin.hpp"
#include "Semaphore.hpp"
#include "extra/ScopedPointer.hpp"
#include "extra/Thread.hpp"


#include "FFTConvolver/TwoStageFFTConvolver.h"
#include "dr_wav.h"


START_NAMESPACE_DISTRHO

#if defined(_MOD_DEVICE_DUO)
static constexpr const size_t headBlockSize = 256;
static constexpr const size_t tailBlockSize = 4096;
#elif defined(_MOD_DEVICE_DWARF)
static constexpr const size_t headBlockSize = 128;
static constexpr const size_t tailBlockSize = 2048;
#else
static constexpr const size_t headBlockSize = 128;
static constexpr const size_t tailBlockSize = 1024;
#endif

// -----------------------------------------------------------------------

class LowPassFilter {
    float c1;
    float c2;
    float c3;
    float r1[2];
    float r2[2];
    float r3[2];
    float r4[2];
    float freq;

public:
    void reset()
    {
        r1[0] = r1[1] = 0.f;
        r2[0] = r2[1] = 0.f;
        r3[0] = r3[1] = 0.f;
        r4[0] = r4[1] = 0.f;
    }

    void setFrequency(const float frequency)
    {
        freq = c2 * frequency;
    }

    void setSampleRate(const float newSampleRate)
    {
        const float c0 = std::min<float>(192000.f, std::max<float>(1.f, newSampleRate));
        c1 = 3.14159274f / c0;
        c2 = 44.0999985f / c0;
        c3 = 1.0f - c2;
        reset();
    }

    inline void process(const float* const input, float* const output, const uint32_t frames)
    {
        static constexpr const float q = 0.06305821314233212f;

        const float f = freq;

        for (uint32_t i = 0; i < frames; ++i)
        {
            r4[0] = f + c3 * r4[1];

            const float t1 = std::tan(c1 * r4[0]);
            const float t2 = (float(input[i]) - r3[1]) * t1;
            const float f3 = t1 + 1.0f;
            const float t4 = 1.0f - t1 / f3;
            const float t5 = (t1
                           * ((r3[1] + (t2 + q * r1[1] * t4) / f3 + r2[1] * (0.f - 1.f / f3)) / (1.f - q * (t1 * t4) / f3) - r1[1])
                           ) / f3;
            const float t6 = r1[1] + t5;

            r1[0] = r1[1] + 2.0f * t5;
            r2[0] = r2[1] + 2.0f * (t1 * (q * t6 - r2[1])) / f3;
            r3[0] = r3[1] + 2.0f * t2 / f3;
            r4[1] = r4[0];
            r1[1] = r1[0];
            r2[1] = r2[0];
            r3[1] = r3[0];
            output[i] = t6;
        }
    }
};

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
    }

    ~TwoStageThreadedConvolver() override
    {
        stop();
    }

    void start()
    {
        startThread(true);
    }

    void stop()
    {
        signalThreadShouldExit();
        semBgProcStart.post();
        stopThread(5000);
    }

protected:
    void startBackgroundProcessing() override
    {
        semBgProcStart.post();
    }

    void waitForBackgroundProcessing() override
    {
        if (isThreadRunning() && !shouldThreadExit())
            semBgProcFinished.wait();
    }

    void run() override
    {
        while (!shouldThreadExit())
        {
            semBgProcStart.wait();

            if (shouldThreadExit())
                break;

            doBackgroundProcessing();
            semBgProcFinished.post();
        }
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
        const float sampleRate = static_cast<float>(getSampleRate());

        lpfL.setSampleRate(sampleRate);
        lpfR.setSampleRate(sampleRate);

        lpfL.setFrequency(parameters[kParameterLowPassFilter]);
        lpfR.setFrequency(parameters[kParameterLowPassFilter]);

        smoothWetLevel.setSampleRate(sampleRate);
        smoothDryLevel.setSampleRate(sampleRate);

        smoothWetLevel.setTimeConstant(0.1f);
        smoothDryLevel.setTimeConstant(0.1f);
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
        return "ISC";
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('O', 'K', 'c', 'r');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter &parameter) override
    {
        switch (index)
        {
        case kParameterWetLevel:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "Wet Level";
            parameter.symbol = "wetlevel";
            parameter.unit = "dB";
            parameter.ranges.def = kParameterDefaults[kParameterWetLevel];
            parameter.ranges.min = -60.f;
            parameter.ranges.max = 0.f;
            {
                ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[1];
                enumValues[0].value = -60.f;
                enumValues[0].label = "Off";
                parameter.enumValues.count = 1;
                parameter.enumValues.values = enumValues;
            }
            break;
        case kParameterDryLevel:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "Dry Level";
            parameter.symbol = "drylevel";
            parameter.unit = "dB";
            parameter.ranges.def = kParameterDefaults[kParameterDryLevel];
            parameter.ranges.min = -60.f;
            parameter.ranges.max = 0.f;
            {
                ParameterEnumerationValue* const enumValues =  new ParameterEnumerationValue[1];
                enumValues[0].value = -60.f;
                enumValues[0].label = "Off";
                parameter.enumValues.count = 1;
                parameter.enumValues.values = enumValues;
            }
            break;
        case kParameterLowPassFilter:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "Low Pass Filter";
            parameter.symbol = "lpf";
            parameter.unit = "Hz";
            parameter.ranges.def = kParameterDefaults[kParameterLowPassFilter];
            parameter.ranges.min = 0.f;
            parameter.ranges.max = 300.f;
            {
                ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[4];
                enumValues[0].value = 0.f;
                enumValues[0].label = "Off";
                enumValues[1].value = 75.f;
                enumValues[1].label = "75";
                enumValues[2].value = 150.f;
                enumValues[2].label = "150";
                enumValues[3].value = 300.f;
                enumValues[3].label = "300";
                parameter.enumValues.count = 4;
                parameter.enumValues.values = enumValues;
            }
            break;
        }
    }

    void initProgramName(uint32_t index, String &programName) override
    {
        switch (index)
        {
        case kProgramDefault:
            programName = "Default";
            break;
        }
    }

    void initState(uint32_t index, State &state) override
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

    void setParameterValue(const uint32_t index, const float value) override
    {
        switch (index)
        {
        case kParameterWetLevel:
            smoothWetLevel.setTarget(std::pow(10.f, 0.05f * value));
            break;
        case kParameterDryLevel:
            smoothDryLevel.setTarget(std::pow(10.f, 0.05f * value));
            break;
        case kParameterLowPassFilter:
            lpfL.setFrequency(value);
            lpfR.setFrequency(value);
            break;
        }

        OneKnobPlugin::setParameterValue(index, value);
    }

    void loadProgram(const uint32_t index) override
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

    void setState(const char *const key, const char *const value) override
    {
        if (std::strcmp(key, "irfile") == 0)
        {
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 impulseResponseSize;

            float* const newImpulseResponse = drwav_open_file_and_read_pcm_frames_f32(
                value, &channels, &sampleRate, &impulseResponseSize, nullptr);
            DISTRHO_SAFE_ASSERT_RETURN(newImpulseResponse != nullptr,);

            loadedFilename = value;

            float* irBufL;
            float* irBufR;
            switch (channels)
            {
            case 1:
                irBufL = newImpulseResponse;
                irBufR = new float[impulseResponseSize];
                std::memcpy(irBufR, irBufL, sizeof(float) * impulseResponseSize);
                break;
            case 2:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i = 0, j = 0; i < impulseResponseSize; ++i)
                {
                    irBufL[i] = newImpulseResponse[j++];
                    irBufR[i] = newImpulseResponse[j++];
                }
                break;
            case 4:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i = 0, j = 0; i < impulseResponseSize; ++i, j += 4)
                {
                    irBufL[i] = newImpulseResponse[j + 0] + newImpulseResponse[j + 2];
                    irBufR[i] = newImpulseResponse[j + 1] + newImpulseResponse[j + 3];
                }
                break;
            default:
                irBufL = new float[impulseResponseSize];
                irBufR = new float[impulseResponseSize];
                for (drwav_uint64 i = 0, j = 0; i < impulseResponseSize; ++i)
                {
                    irBufL[i] = irBufR[i] = newImpulseResponse[j];
                    j += channels;
                }
                break;
            }

            ScopedPointer<TwoStageThreadedConvolver> newConvolverL, newConvolverR;

            newConvolverL = new TwoStageThreadedConvolver();
            newConvolverL->init(headBlockSize, tailBlockSize, irBufL, impulseResponseSize);
            newConvolverL->start();

            newConvolverR = new TwoStageThreadedConvolver();
            newConvolverR->init(headBlockSize, tailBlockSize, irBufR, impulseResponseSize);
            newConvolverR->start();

            {
                const MutexLocker cml(mutex);
                convolverL.swapWith(newConvolverL);
                convolverR.swapWith(newConvolverR);
            }

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
        const uint32_t bufSize = getBufferSize();
        inlineProcBufL = new float[bufSize];
        inlineProcBufR = new float[bufSize];

        lpfL.reset();
        lpfR.reset();

        smoothWetLevel.clearToTarget();
        smoothDryLevel.clearToTarget();

        OneKnobPlugin::activate();
    }

    void deactivate() override
    {
        delete[] inlineProcBufL;
        delete[] inlineProcBufR;
        inlineProcBufL = inlineProcBufR = nullptr;
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        // optimize for non-denormal usage
        for (uint32_t i = 0; i < frames; ++i)
        {
            if (!std::isfinite(inputs[0][i]))
                __builtin_unreachable();
            if (!std::isfinite(inputs[1][i]))
                __builtin_unreachable();
            if (!std::isfinite(outputs[0][i]))
                __builtin_unreachable();
            if (!std::isfinite(outputs[1][i]))
                __builtin_unreachable();
        }

        const float* const inL = inlineProcBufL;
        const float* const inR = inlineProcBufR;
        /* */ float* const outL = outputs[0];
        /* */ float* const outR = outputs[1];

        const int lpf = static_cast<int>(parameters[kParameterLowPassFilter] + 0.5f);

        if (lpf == 0)
        {
            std::memcpy(inlineProcBufL, inputs[0], sizeof(float) * frames);
            std::memcpy(inlineProcBufR, inputs[1], sizeof(float) * frames);
        }
        else
        {
            lpfL.process(inputs[0], inlineProcBufL, frames);
            lpfR.process(inputs[1], inlineProcBufR, frames);
        }

        float wetLevel, dryLevel;
       #ifdef HAVE_OPENGL
        float tmp1 = lineGraphHighest1;
        float tmp2 = lineGraphHighest2;
       #endif

        const MutexTryLocker cmtl(mutex);

        if (cmtl.wasLocked())
        {
            TwoStageThreadedConvolver *const convL = convolverL.get();
            TwoStageThreadedConvolver *const convR = convolverR.get();

            if (convL != nullptr && convR != nullptr)
            {
                convL->process(inL, outL, frames);
                convR->process(inR, outR, frames);

                for (uint32_t i = 0; i < frames; ++i) {
                    wetLevel = smoothWetLevel.next();
                    dryLevel = smoothDryLevel.next();

                    if (wetLevel <= 0.001f)
                    {
                        outL[i] = outR[i] = 0.f;
                    }
                    else
                    {
                        outL[i] *= wetLevel;
                        outR[i] *= wetLevel;
                    }

                   #ifdef HAVE_OPENGL
                    tmp2 = std::max(tmp2, std::abs(outL[i]));
                    tmp2 = std::max(tmp2, std::abs(outR[i]));
                   #endif

                    if (dryLevel > 0.001f)
                    {
                        outL[i] += inL[i] * dryLevel;
                        outR[i] += inR[i] * dryLevel;
                       #ifdef HAVE_OPENGL
                        tmp1 = std::max(tmp1, std::abs(inL[i] * dryLevel));
                        tmp1 = std::max(tmp1, std::abs(inR[i] * dryLevel));
                       #endif
                    }

                   #ifdef HAVE_OPENGL
                    if (++lineGraphFrameCounter == lineGraphFrameToReset)
                    {
                        lineGraphFrameCounter = 0;
                        setMeters(tmp1, tmp2);
                        tmp1 = tmp2 = 0.f;
                    }
                   #endif
                }

               #ifdef HAVE_OPENGL
                lineGraphHighest1 = tmp1;
                lineGraphHighest2 = tmp2;
               #endif

                return;
            }
        }

        for (uint32_t i = 0; i < frames; ++i)
        {
            smoothWetLevel.next();
            dryLevel = smoothDryLevel.next();

            outL[i] = inL[i] * dryLevel;
            outR[i] = inR[i] * dryLevel;

           #ifdef HAVE_OPENGL
            tmp1 = std::max(tmp1, std::abs(outL[i]));
            tmp1 = std::max(tmp1, std::abs(outR[i]));

            if (++lineGraphFrameCounter == lineGraphFrameToReset)
            {
                lineGraphFrameCounter = 0;
                setMeters(tmp1, tmp2);
                tmp1 = tmp2 = 0.f;
            }
           #endif
        }

       #ifdef HAVE_OPENGL
        lineGraphHighest1 = tmp1;
        lineGraphHighest2 = tmp2;
       #endif
    }

    void sampleRateChanged(const double newSampleRate) override
    {
        lpfL.setSampleRate(newSampleRate);
        lpfR.setSampleRate(newSampleRate);

        lpfL.setFrequency(parameters[kParameterLowPassFilter]);
        lpfR.setFrequency(parameters[kParameterLowPassFilter]);

        smoothWetLevel.setSampleRate(newSampleRate);
        smoothDryLevel.setSampleRate(newSampleRate);

        // reload file
        if (char *const filename = loadedFilename.getAndReleaseBuffer())
        {
            setState("irfile", filename);
            std::free(filename);
        }
    }

  // -------------------------------------------------------------------

private:
    ScopedPointer<TwoStageThreadedConvolver> convolverL, convolverR;
    LowPassFilter lpfL, lpfR;
    Mutex mutex;
    String loadedFilename;

    // smoothed parameters
    LinearSmoother smoothWetLevel;
    LinearSmoother smoothDryLevel;

    // if doing inline processing, copy buffers here before convolution
    float *inlineProcBufL = nullptr;
    float *inlineProcBufR = nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobConvolutionReverbPlugin)
};

// -----------------------------------------------------------------------

Plugin *createPlugin()
{
    return new OneKnobConvolutionReverbPlugin();
}

END_NAMESPACE_DISTRHO
