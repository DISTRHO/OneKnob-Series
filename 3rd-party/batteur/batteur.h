#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined BATTEUR_EXPORT_SYMBOLS
  #if defined _WIN32
    #define BATTEUR_EXPORTED_API __declspec(dllexport)
  #else
    #define BATTEUR_EXPORTED_API __attribute__ ((visibility ("default")))
  #endif
#else
  #define BATTEUR_EXPORTED_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct batteur_beat_t batteur_beat_t;
typedef struct batteur_player_t batteur_player_t;
typedef void (*batteur_note_cb_t)(int delay, uint8_t number, float value, void* cbdata);
typedef enum { 
  BATTEUR_STOPPED = 0,
  BATTEUR_INTRO,
  BATTEUR_PLAYING,
  BATTEUR_FILL_IN,
  BATTEUR_NEXT,
  BATTEUR_ENDING
} batteur_status_t;

BATTEUR_EXPORTED_API  batteur_beat_t* batteur_load_beat(const char* filename);
BATTEUR_EXPORTED_API  batteur_beat_t* batteur_load_beat_from_string(const char* filename, const char* string);
BATTEUR_EXPORTED_API  void batteur_free_beat(batteur_beat_t* beat);
BATTEUR_EXPORTED_API  const char* batteur_get_beat_name(batteur_beat_t* beat);
BATTEUR_EXPORTED_API  const char* batteur_get_part_name(batteur_beat_t* beat, int part_index);
BATTEUR_EXPORTED_API  int batteur_get_total_parts(batteur_beat_t* beat);
BATTEUR_EXPORTED_API  int batteur_get_total_fills(batteur_beat_t* beat, int part_index);
BATTEUR_EXPORTED_API  int batteur_get_time_numerator(batteur_beat_t* beat);
BATTEUR_EXPORTED_API  int batteur_get_time_denominator(batteur_beat_t* beat);

BATTEUR_EXPORTED_API  batteur_player_t* batteur_new();
BATTEUR_EXPORTED_API  void batteur_free(batteur_player_t* player);
BATTEUR_EXPORTED_API  bool batteur_load(batteur_player_t* player, batteur_beat_t* beat);
BATTEUR_EXPORTED_API  void batteur_set_sample_rate(batteur_player_t* player, double sample_rate);
BATTEUR_EXPORTED_API  void batteur_note_cb(batteur_player_t* player, batteur_note_cb_t callback, void* cbdata);
BATTEUR_EXPORTED_API  void batteur_set_tempo(batteur_player_t* player, double bpm);
BATTEUR_EXPORTED_API  double batteur_get_tempo(batteur_player_t* player);
BATTEUR_EXPORTED_API  batteur_beat_t* batteur_get_current_beat(batteur_player_t* player);
BATTEUR_EXPORTED_API  void batteur_tick(batteur_player_t* player, int sample_count);
BATTEUR_EXPORTED_API  void batteur_fill_in(batteur_player_t* player);
BATTEUR_EXPORTED_API  void batteur_next(batteur_player_t* player);
BATTEUR_EXPORTED_API  void batteur_stop(batteur_player_t* player);
BATTEUR_EXPORTED_API  void batteur_start(batteur_player_t* player);
BATTEUR_EXPORTED_API  void batteur_all_off(batteur_player_t* player);
BATTEUR_EXPORTED_API  bool batteur_playing(batteur_player_t* player);
BATTEUR_EXPORTED_API  batteur_status_t batteur_get_status(batteur_player_t* player);
BATTEUR_EXPORTED_API  double batteur_get_bar_position(batteur_player_t* player);
BATTEUR_EXPORTED_API  int batteur_get_part_index(batteur_player_t* player);
BATTEUR_EXPORTED_API  int batteur_get_fill_index(batteur_player_t* player);

#ifdef __cplusplus
}
#endif
