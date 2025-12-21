#include "batteur.h"
#include "BeatDescription.h"
#include "Player.h"

#ifdef __cplusplus
extern "C" {
#endif

batteur_beat_t* batteur_load_beat(const char* filename)
{
    std::error_code ec;
    auto beat = batteur::BeatDescription::buildFromFile(filename, ec);
    if (ec)
        return NULL;

    return reinterpret_cast<batteur_beat_t*>(beat.release());
}

batteur_beat_t* batteur_load_beat_from_string(const char* filename, const char* string)
{
    std::error_code ec;
    auto beat = batteur::BeatDescription::buildFromString(filename, string, ec);
    if (ec)
        return NULL;

    return reinterpret_cast<batteur_beat_t*>(beat.release());
}

void batteur_free_beat(batteur_beat_t* beat)
{
    delete reinterpret_cast<batteur::BeatDescription*>(beat);
}

const char* batteur_get_beat_name(batteur_beat_t* beat)
{
    if (!beat)
        return {};

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    return self->name.c_str();
}

const char* batteur_get_part_name(batteur_beat_t* beat, int part_index)
{
    if (!beat)
        return {};

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    const auto numParts = static_cast<int>(self->parts.size());
    if (part_index < 0 || part_index >= numParts)
        return {};

    return self->parts[part_index].name.c_str();
}

int batteur_get_total_parts(batteur_beat_t* beat)
{
    if (!beat)
        return 0;

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    return static_cast<int>(self->parts.size());
}

int batteur_get_total_fills(batteur_beat_t* beat, int part_index)
{
    if (!beat)
        return 0;

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    const auto numParts = static_cast<int>(self->parts.size());
    if (part_index < 0 || part_index >= numParts)
        return 0;

    return static_cast<int>(self->parts[part_index].fills.size());
}

int batteur_get_time_numerator(batteur_beat_t* beat)
{
    if (!beat)
        return 0;

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    return self->signature.num;
}

int batteur_get_time_denominator(batteur_beat_t* beat)
{
    if (!beat)
        return 0;

    auto self = reinterpret_cast<batteur::BeatDescription*>(beat);
    return self->signature.denom;
}



batteur_player_t* batteur_new()
{
    return reinterpret_cast<batteur_player_t*>(new batteur::Player);
}

void batteur_free(batteur_player_t* player)
{
    delete reinterpret_cast<batteur::Player*>(player);
}

bool batteur_load(batteur_player_t* player, batteur_beat_t* beat)
{
    if (!player || !beat)
        return false;

    auto self = reinterpret_cast<batteur::Player*>(player);
    auto description = reinterpret_cast<batteur::BeatDescription*>(beat);    
    return self->loadBeatDescription(*description);
}

void batteur_set_sample_rate(batteur_player_t* player, double sample_rate)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->setSampleRate(sample_rate);
}

void batteur_note_cb(batteur_player_t* player, batteur_note_cb_t callback, void* cbdata)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->setNoteCallback([=](int delay, uint8_t number, float velocity) {
        callback(delay, number, velocity, cbdata);
    });
}

void batteur_set_tempo(batteur_player_t* player, double bpm)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->setTempo(bpm);
}

void batteur_tick(batteur_player_t* player, int sample_count)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->tick(sample_count);
}

void batteur_fill_in(batteur_player_t* player)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->fillIn();
}

void batteur_next(batteur_player_t* player)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->next();
}

void batteur_stop(batteur_player_t* player)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->stop();
}

void batteur_start(batteur_player_t* player)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->start();
}

void batteur_all_off(batteur_player_t* player)
{
    if (!player)
        return;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    self->allOff();
}

bool batteur_playing(batteur_player_t* player)
{
    if (!player)
        return false;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return self->isPlaying();
}

double batteur_get_tempo(batteur_player_t* player)
{
    if (!player)
        return 120.0;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return self->getTempo();
}

batteur_beat_t* batteur_get_current_beat(batteur_player_t* player)
{
    if (!player)
        return NULL;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return (batteur_beat_t*)self->getBeatDescription();
}

batteur_status_t batteur_get_status(batteur_player_t* player)
{
    using namespace batteur;

    if (!player)
        return BATTEUR_STOPPED;
    
    auto self = reinterpret_cast<Player*>(player);
    switch (self->getState()) {
    case Player::State::Stopped: return BATTEUR_STOPPED;
    case Player::State::Playing: return BATTEUR_PLAYING;
    case Player::State::Fill: return BATTEUR_FILL_IN;
    case Player::State::Next: return BATTEUR_NEXT;
    case Player::State::Intro: return BATTEUR_INTRO;
    case Player::State::Ending: return BATTEUR_ENDING;
    }

    return BATTEUR_STOPPED;
}

double batteur_get_bar_position(batteur_player_t* player)
{
    if (!player)
        return 0.0;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return self->getBarPosition();
}

int batteur_get_part_index(batteur_player_t* player)
{
    if (!player)
        return 0;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return self->getPartIndex();
}

int batteur_get_fill_index(batteur_player_t* player)
{
    if (!player)
        return 0;
    
    auto self = reinterpret_cast<batteur::Player*>(player);
    return self->getFillIndex();
}

#ifdef __cplusplus
}
#endif
