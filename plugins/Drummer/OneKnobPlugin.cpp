/*
 * DISTRHO OneKnob Drummer
 * Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
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

#include "BeatDescription.h"
#include "Player.h"

#include "fluidsynth.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class OneKnobDrummerPlugin : public OneKnobPlugin
{
    batteur::Player* const player = new batteur::Player;
    std::unique_ptr<batteur::BeatDescription> beats[kNumStyles];
    uint32_t lastBeatStyle = kDefaultStyle;
    double lastKnownTempo = 120.0;

    fluid_settings_t* settings = nullptr;
    fluid_synth_t*    synth = nullptr;
    int               synthId;

    fluid_midi_event_t* fmidi_event = nullptr;
    int fmidi_last_offset = 0;
    float* fmidi_out1 = nullptr;
    float* fmidi_out2 = nullptr;

public:
    OneKnobDrummerPlugin()
        : OneKnobPlugin()
    {
        init();

        const double sampleRate = getSampleRate();

        player->setSampleRate(sampleRate);
        player->setNoteCallback([=](int delay, uint8_t number, float velocity) {
            // render audio until this note
            if (delay > fmidi_last_offset) {
                fluid_synth_write_float(
                    synth,
                    delay - fmidi_last_offset,
                    fmidi_out1 + fmidi_last_offset,
                    0,
                    1,
                    fmidi_out2 + fmidi_last_offset,
                    0,
                    1);
                fmidi_last_offset = delay;
            }

            fluid_midi_event_set_type(fmidi_event, d_isZero(velocity) ? 0x80 : 0x90);
            fluid_midi_event_set_key(fmidi_event, number);
            fluid_midi_event_set_value(fmidi_event, std::clamp<uint8_t>(velocity * 127, 0, 127));

            fluid_synth_handle_midi_event(synth, fmidi_event);
        });

        const String bundlePath(getBundlePath());
        std::error_code ec;
        for (uint32_t i = 0; i < kNumStyles; ++i)
        {
            const String path = bundlePath
                              + DISTRHO_OS_SEP_STR "beats" DISTRHO_OS_SEP_STR
                              + kStyleNames[i] + ".json";
            d_stdout("Loading %s", path.buffer());
            beats[i] = batteur::BeatDescription::buildFromFile(path.buffer(), ec);
        }

        player->loadBeatDescription(*beats[kDefaultStyle]);

        /* initialize fluid synth */
        settings = new_fluid_settings();
        DISTRHO_SAFE_ASSERT_RETURN(settings != nullptr,);

        fluid_settings_setnum(settings, "synth.sample-rate", sampleRate);
        fluid_settings_setint(settings, "synth.threadsafe-api", 0);
        // fluid_settings_setstr(settings, "synth.midi-bank-select", "mma");

        synth = new_fluid_synth(settings);
        DISTRHO_SAFE_ASSERT_RETURN(synth != nullptr,);

        fluid_synth_set_gain(synth, 1.f);
        fluid_synth_set_polyphony(synth, 256);
        fluid_synth_set_sample_rate(synth, sampleRate);

        fluid_synth_set_reverb_on(synth, false);
        fluid_synth_set_chorus_on(synth, false);

        fmidi_event = new_fluid_midi_event();
        DISTRHO_SAFE_ASSERT_RETURN(fmidi_event != nullptr,);

        fluid_midi_event_set_channel(fmidi_event, 0);

        synthId = fluid_synth_sfload(synth, bundlePath + DISTRHO_OS_SEP_STR "Black_Pearl_4_LV2.sf2", 1);
        DISTRHO_SAFE_ASSERT_RETURN(synthId != FLUID_FAILED,);

        // fluid_synth_program_select(synth, 0, synthId, 0, 0);
    }

    ~OneKnobDrummerPlugin() override
    {
        delete_fluid_midi_event(fmidi_event);

        delete_fluid_synth(synth);
        delete_fluid_settings(settings);

        delete player;
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
        return d_cconst('O', 'K', 'd', 'r');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
        case kParameterBypass:
            parameter.initDesignation(kParameterDesignationBypass);
            break;
        case kParameterStyle:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name   = "Style";
            parameter.symbol = "style";
            parameter.ranges.def = kDefaultStyle;
            parameter.ranges.max = kNumStyles - 1;
            if (ParameterEnumerationValue* const values = new ParameterEnumerationValue[kNumStyles])
            {
                parameter.enumValues.count = kNumStyles;
                parameter.enumValues.values = values;
                parameter.enumValues.restrictedMode = true;

                for (uint32_t i = 0; i < kNumStyles; ++i)
                {
                    values[i].label = kStyleNames[i];
                    values[i].value = i;
                }
            }
            break;
        case kParameterActive:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger | kParameterIsBoolean;
            parameter.name   = "Active";
            parameter.symbol = "active";
            parameter.ranges.def = 1.f;
            break;
        case kParameterFillIn:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger | kParameterIsTrigger;
            parameter.name   = "Fill In";
            parameter.symbol = "fill_in";
            break;
        case kParameterNext:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger | kParameterIsTrigger;
            parameter.name   = "Next";
            parameter.symbol = "next";
            break;
        case kParameterStatus:
            parameter.hints  = kParameterIsAutomatable | kParameterIsInteger | kParameterIsOutput;
            parameter.name   = "Status";
            parameter.symbol = "status";
            parameter.ranges.max = kParameterRanges[kParameterStatus].max;
            if (ParameterEnumerationValue* const values = new ParameterEnumerationValue[6])
            {
                parameter.enumValues.count = 6;
                parameter.enumValues.values = values;
                parameter.enumValues.restrictedMode = true;

                values[0].label = "Stopped";
                values[0].value = 0.0f;
                values[1].label = "Intro";
                values[1].value = 1.0f;
                values[2].label = "Playing";
                values[2].value = 2.0f;
                values[3].label = "Fill";
                values[3].value = 3.0f;
                values[4].label = "Next";
                values[4].value = 4.0f;
                values[5].label = "Ending";
                values[5].value = 5.0f;
            }
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

        switch (index)
        {
        case kParameterBypass:
            if (value > 0.5f)
            {
                player->stop();
                player->allOff();
            }
            else if (parameters[kParameterActive] > 0.5f)
            {
                player->start();
            }
            break;
        case kParameterStyle:
            player->allOff();
            player->loadBeatDescription(
                *beats[std::clamp(d_roundToUnsignedInt(value), 0u, kNumStyles - 1)]);
            break;
        case kParameterActive:
            if (value > 0.5f)
                player->start();
            else if (player->isPlaying())
                player->stop();
            break;
        case kParameterFillIn:
            if (value > 0.5f)
                player->fillIn();
            break;
        case kParameterNext:
            if (value > 0.5f)
                player->next();
            break;
        }
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
        // activate();
    }

    // -------------------------------------------------------------------
    // Process

    void stop()
    {
        // fluid_synth_all_notes_off(synth, -1);
        // fluid_synth_all_sounds_off(synth, -1);

        player->stop();
        player->allOff();
    }

    void activate() override
    {
        OneKnobPlugin::activate();

        const uint32_t bufferSize = getBufferSize();
        fmidi_out1 = new float[bufferSize];
        fmidi_out2 = new float[bufferSize];
    }

    void deactivate() override
    {
        delete[] fmidi_out1;
        delete[] fmidi_out2;
    }

    void run(const float** const inputs, float** const outputs, const uint32_t frames) override
    {
        const float* in1  = inputs[0];
        const float* in2  = inputs[1];
        float*       out1 = outputs[0];
        float*       out2 = outputs[1];

        fmidi_last_offset = 0;

        const TimePosition& timePos(getTimePosition());
        player->setTempo(timePos.bbt.beatsPerMinute);

        player->tick(frames);

        const batteur::Player::State state = player->getState();
        parameters[kParameterStatus] = static_cast<float>(state);

        // enforce state
        if (state == batteur::Player::State::Stopped
            && d_isZero(parameters[kParameterBypass])
            && d_isNotZero(parameters[kParameterActive]))
            player->start();

        // render audio until end of buffer
        if (fmidi_last_offset < frames - 1)
        {
            fluid_synth_write_float(
                synth,
                frames - fmidi_last_offset,
                fmidi_out1 + fmidi_last_offset,
                0,
                1,
                fmidi_out2 + fmidi_last_offset,
                0,
                1);
        }

        if (out1 != in1)
        {
            for (uint32_t i = 0; i < frames; ++i)
                out1[i] = in1[i] + fmidi_out1[i];
        }
        else
        {
            for (uint32_t i = 0; i < frames; ++i)
                out1[i] += fmidi_out1[i];
        }

        if (out2 != in2)
        {
            for (uint32_t i = 0; i < frames; ++i)
                out2[i] = in2[i] + fmidi_out2[i];
        }
        else
        {
            for (uint32_t i = 0; i < frames; ++i)
                out2[i] += fmidi_out2[i];
        }
    }


    // -------------------------------------------------------------------

private:

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobDrummerPlugin)
};

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new OneKnobDrummerPlugin();
}

END_NAMESPACE_DISTRHO
