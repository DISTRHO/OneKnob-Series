/*
 * DISTRHO OneKnob Convolution Reverb
 * Copyright (C) 2022-2023 Filipe Coelho <falktx@falktx.com>
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

#include "OneKnobPlugin.hpp"
#include "Korg35Filters.hpp"

#include "Semaphore.hpp"
#include "extra/ScopedPointer.hpp"
#include "extra/Thread.hpp"

#include "dr_flac.h"
#include "dr_wav.h"
// -Wunused-variable
#include "r8brain/CDSPResampler.h"

// must be last
#include "TwoStageThreadedConvolver.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobConvolutionReverbPlugin : public OneKnobPlugin
{
public:
    OneKnobConvolutionReverbPlugin()
        : OneKnobPlugin()
    {
        const float sampleRate = static_cast<float>(getSampleRate());

        korgFilterL.setSampleRate(sampleRate);
        korgFilterR.setSampleRate(sampleRate);

        korgFilterL.setFrequency(kParameterRanges[kParameterHighPassFilter].def);
        korgFilterR.setFrequency(kParameterRanges[kParameterHighPassFilter].def);

        smoothDryLevel.setSampleRate(sampleRate);
        smoothWetLevel.setSampleRate(sampleRate);

        smoothDryLevel.setTimeConstant(0.1f);
        smoothWetLevel.setTimeConstant(0.1f);

        smoothDryLevel.setTargetValue(std::pow(10.f, 0.05f * kParameterRanges[kParameterDryLevel].def));
        smoothWetLevel.setTargetValue(std::pow(10.f, 0.05f * kParameterRanges[kParameterWetLevel].def));
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

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterDryLevel:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "Dry Level";
            parameter.symbol = "drylevel";
            parameter.unit = "dB";
            parameter.ranges.def = kParameterRanges[kParameterDryLevel].def;
            parameter.ranges.min = kParameterRanges[kParameterDryLevel].min;
            parameter.ranges.max = kParameterRanges[kParameterDryLevel].max;
            {
                ParameterEnumerationValue* const enumValues =  new ParameterEnumerationValue[1];
                enumValues[0].value = kParameterRanges[kParameterDryLevel].min;
                enumValues[0].label = "Off";
                parameter.enumValues.count = 1;
                parameter.enumValues.values = enumValues;
            }
            break;
        case kParameterWetLevel:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "Wet Level";
            parameter.symbol = "wetlevel";
            parameter.unit = "dB";
            parameter.ranges.def = kParameterRanges[kParameterWetLevel].def;
            parameter.ranges.min = kParameterRanges[kParameterWetLevel].min;
            parameter.ranges.max = kParameterRanges[kParameterWetLevel].max;
            {
                ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[1];
                enumValues[0].value = kParameterRanges[kParameterWetLevel].min;
                enumValues[0].label = "Off";
                parameter.enumValues.count = 1;
                parameter.enumValues.values = enumValues;
            }
            break;
        case kParameterHighPassFilter:
            parameter.hints = kParameterIsAutomatable;
            parameter.name = "High Pass Filter";
            parameter.symbol = "hpf";
            parameter.unit = "Hz";
            parameter.ranges.def = kParameterRanges[kParameterHighPassFilter].def;
            parameter.ranges.min = kParameterRanges[kParameterHighPassFilter].min;
            parameter.ranges.max = kParameterRanges[kParameterHighPassFilter].max;
            {
                ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[1];
                enumValues[0].value = 0.f;
                enumValues[0].label = "Off";
                parameter.enumValues.count = 1;
                parameter.enumValues.values = enumValues;
            }
            break;
        case kParameterTrails:
            parameter.hints = kParameterIsAutomatable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name = "Trails";
            parameter.symbol = "trails";
            parameter.ranges.def = kParameterRanges[kParameterTrails].def;
            parameter.ranges.min = kParameterRanges[kParameterTrails].min;
            parameter.ranges.max = kParameterRanges[kParameterTrails].max;
            break;
        case kParameterBypass:
            parameter.initDesignation(kParameterDesignationBypass);
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
        case kParameterDryLevel:
            if (!bypassed)
                smoothDryLevel.setTargetValue(std::pow(10.f, 0.05f * value));
            break;
        case kParameterWetLevel:
            if (!bypassed)
                smoothWetLevel.setTargetValue(std::pow(10.f, 0.05f * value));
            break;
        case kParameterHighPassFilter:
            korgFilterL.setFrequency(value);
            korgFilterR.setFrequency(value);
            break;
        case kParameterTrails:
            trails = value > 0.5f;
            if (bypassed)
                smoothWetLevel.setTargetValue(trails ? std::pow(10.f, 0.05f * parameters[kParameterWetLevel]) : 0.f);
            break;
        case kParameterBypass:
            bypassed = value > 0.5f;
            if (bypassed)
            {
                smoothDryLevel.setTargetValue(1.f);
                smoothWetLevel.setTargetValue(trails ? std::pow(10.f, 0.05f * parameters[kParameterWetLevel]) : 0.f);
            }
            else
            {
                korgFilterL.reset();
                korgFilterR.reset();
                smoothDryLevel.setTargetValue(std::pow(10.f, 0.05f * parameters[kParameterDryLevel]));
                smoothWetLevel.setTargetValue(std::pow(10.f, 0.05f * parameters[kParameterWetLevel]));
            }
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

        korgFilterL.reset();
        korgFilterR.reset();

        smoothDryLevel.clearToTargetValue();
        smoothWetLevel.clearToTargetValue();
    }

    void setState(const char* const key, const char* const value) override
    {
        if (std::strcmp(key, "irfile") == 0)
        {
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 numFrames;
            const size_t valuelen = std::strlen(value);

            ScopedPointer<TwoStageThreadedConvolver> newConvolverL, newConvolverR;

            if (valuelen <= 5)
            {
                const MutexLocker cml(mutex);
                convolverL.swapWith(newConvolverL);
                convolverR.swapWith(newConvolverR);
                return;
            }

            float* ir;
            if (::strncasecmp(value + (std::max(size_t(0), valuelen - 5u)), ".flac", 5) == 0)
                ir = drflac_open_file_and_read_pcm_frames_f32(value, &channels, &sampleRate, &numFrames, nullptr);
            else
                ir = drwav_open_file_and_read_pcm_frames_f32(value, &channels, &sampleRate, &numFrames, nullptr);
            DISTRHO_SAFE_ASSERT_RETURN(ir != nullptr,);

            loadedFilename = value;

            float* irBufL;
            float* irBufR;
            switch (channels)
            {
            case 1:
                irBufL = irBufR = ir;
                break;
            case 2:
                irBufL = new float[numFrames];
                irBufR = new float[numFrames];
                for (drwav_uint64 i = 0, j = 0; i < numFrames; ++i)
                {
                    irBufL[i] = ir[j++];
                    irBufR[i] = ir[j++];
                }
                break;
            case 4:
                irBufL = new float[numFrames];
                irBufR = new float[numFrames];
                for (drwav_uint64 i = 0, j = 0; i < numFrames; ++i, j += 4)
                {
                    irBufL[i] = ir[j + 0] + ir[j + 2];
                    irBufR[i] = ir[j + 1] + ir[j + 3];
                }
                break;
            default:
                irBufL = new float[numFrames];
                irBufR = new float[numFrames];
                for (drwav_uint64 i = 0, j = 0; i < numFrames; ++i)
                {
                    irBufL[i] = irBufR[i] = ir[j];
                    j += channels;
                }
                break;
            }

            if (sampleRate != getSampleRate())
            {
                r8b::CDSPResampler16IR resampler(sampleRate, getSampleRate(), numFrames);
                const int numResampledFrames = resampler.getMaxOutLen(0);
                DISTRHO_SAFE_ASSERT_RETURN(numResampledFrames > 0,);

                // left channel, always present
                float* const irBufResampledL = new float[numResampledFrames];
                resampler.oneshot(irBufL, numFrames, irBufResampledL, numResampledFrames);
                delete[] irBufL;
                irBufL = irBufResampledL;

                // right channel, optional
                if (irBufL != irBufR)
                {
                    float* const irBufResampledR = new float[numResampledFrames];
                    resampler.oneshot(irBufR, numFrames, irBufResampledR, numResampledFrames);
                    delete[] irBufR;
                    irBufR = irBufResampledR;
                }

                numFrames = numResampledFrames;
            }

            newConvolverL = new TwoStageThreadedConvolver();
            newConvolverL->init(irBufL, numFrames);

            newConvolverR = new TwoStageThreadedConvolver();
            newConvolverR->init(irBufR, numFrames);

            {
                const MutexLocker cml(mutex);
                convolverL.swapWith(newConvolverL);
                convolverR.swapWith(newConvolverR);
            }

            if (irBufL != ir)
                delete[] irBufL;
            if (irBufR != irBufL)
                delete[] irBufR;

            drwav_free(ir, nullptr);
            return;
        }

        OneKnobPlugin::setState(key, value);
    }

    // -------------------------------------------------------------------
    // Process

    void activate() override
    {
        const uint32_t bufSize = bufferSize = getBufferSize();

        highpassBufL = new float[bufSize];
        highpassBufR = new float[bufSize];
        inplaceProcBufL = new float[bufSize];
        inplaceProcBufR = new float[bufSize];

        korgFilterL.reset();
        korgFilterR.reset();

        smoothDryLevel.clearToTargetValue();
        smoothWetLevel.clearToTargetValue();

        OneKnobPlugin::activate();
    }

    void deactivate() override
    {
        delete[] highpassBufL;
        delete[] highpassBufR;
        delete[] inplaceProcBufL;
        delete[] inplaceProcBufR;
        bufferSize = 0;
        highpassBufL = highpassBufR = nullptr;
        inplaceProcBufL = inplaceProcBufR = nullptr;
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

        for (uint32_t offset = 0; offset < frames; offset += bufferSize)
            run(inputs, outputs, std::min(frames - offset, bufferSize), offset);
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames, const uint32_t offset)
    {
        const float* const inL = inputs[0] + offset;
        const float* const inR = inputs[1] + offset;
        /* */ float* const outL = outputs[0] + offset;
        /* */ float* const outR = outputs[1] + offset;

        const float* dryBufL = inL;
        const float* dryBufR = inR;

        const int hpf = static_cast<int>(parameters[kParameterHighPassFilter] + 0.5f);

        if (bypassed)
        {
            std::memset(highpassBufL, 0, sizeof(float) * frames);
            std::memset(highpassBufR, 0, sizeof(float) * frames);
        }
        else if (hpf == 0)
        {
            std::memcpy(highpassBufL, inL, sizeof(float) * frames);
            std::memcpy(highpassBufR, inR, sizeof(float) * frames);
        }
        else
        {
            korgFilterL.processHighPass(inL, highpassBufL, frames);
            korgFilterR.processHighPass(inR, highpassBufR, frames);
        }

        if (outL == inL)
        {
            dryBufL = inplaceProcBufL;
            std::memcpy(inplaceProcBufL, inL, sizeof(float) * frames);
        }

        if (outR == inR)
        {
            dryBufR = inplaceProcBufR;
            std::memcpy(inplaceProcBufR, inR, sizeof(float) * frames);
        }

        float wetLevel, dryLevel;
       #ifdef HAVE_OPENGL
        float tmp1 = lineGraphHighest1;
        float tmp2 = lineGraphHighest2;
       #endif

        const MutexTryLocker cmtl(mutex);

        if (cmtl.wasLocked())
        {
            TwoStageThreadedConvolver* const convL = convolverL.get();
            TwoStageThreadedConvolver* const convR = convolverR.get();

            if (convL != nullptr && convR != nullptr)
            {
                convL->process(highpassBufL, outL, frames);
                convR->process(highpassBufR, outR, frames);

                for (uint32_t i = 0; i < frames; ++i)
                {
                    dryLevel = smoothDryLevel.next();
                    wetLevel = smoothWetLevel.next();

                    if (wetLevel <= 0.001f)
                    {
                        outL[i] = outR[i] = 0.f;
                    }
                    else
                    {
                        outL[i] *= wetLevel;
                        outR[i] *= wetLevel;
                       #ifdef HAVE_OPENGL
                        tmp2 = std::max(tmp2, std::abs(outL[i]));
                        tmp2 = std::max(tmp2, std::abs(outR[i]));
                       #endif
                    }

                    if (dryLevel > 0.001f)
                    {
                        outL[i] += dryBufL[i] * dryLevel;
                        outR[i] += dryBufR[i] * dryLevel;
                       #ifdef HAVE_OPENGL
                        tmp1 = std::max(tmp1, std::abs(dryBufL[i] * dryLevel));
                        tmp1 = std::max(tmp1, std::abs(dryBufR[i] * dryLevel));
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

            outL[i] = dryBufL[i] * dryLevel;
            outR[i] = dryBufR[i] * dryLevel;

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
        korgFilterL.setSampleRate(newSampleRate);
        korgFilterR.setSampleRate(newSampleRate);

        smoothDryLevel.setSampleRate(newSampleRate);
        smoothWetLevel.setSampleRate(newSampleRate);

        // reload file
        if (char* const filename = loadedFilename.getAndReleaseBuffer())
        {
            setState("irfile", filename);
            std::free(filename);
        }
    }

  // -------------------------------------------------------------------

private:
    ScopedPointer<TwoStageThreadedConvolver> convolverL, convolverR;
    Korg35Filter korgFilterL, korgFilterR;
    Mutex mutex;
    String loadedFilename;

    bool bypassed = false;
    bool trails = true;
    uint32_t bufferSize = 0;

    // smoothed parameters
    LinearValueSmoother smoothDryLevel;
    LinearValueSmoother smoothWetLevel;

    // buffers for placing highpass signal before convolution
    float* highpassBufL = nullptr;
    float* highpassBufR = nullptr;

    // if doing inline processing, copy buffers here before convolution
    float* inplaceProcBufL = nullptr;
    float* inplaceProcBufR = nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobConvolutionReverbPlugin)
};

// -----------------------------------------------------------------------

Plugin *createPlugin()
{
    return new OneKnobConvolutionReverbPlugin();
}

END_NAMESPACE_DISTRHO
