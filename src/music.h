#include "stm32f0xx.h"
#include <stdint.h>
#include "midi.h"
#include "midiplay.h"
#define VOICES 15

// An array of "voices".  Each voice can be used to play a different note.
// Each voice can be associated with a channel (explained later).
// Each voice has a step size and an offset into the wave table.
struct {
    uint8_t in_use;
    uint8_t note;
    uint8_t chan;
    uint8_t volume;
    int     step;
    int     offset;
} voice[VOICES];
void TIM6_DAC_IRQHandler(void);
void init_dac(void);
void init_tim6(void);
void note_off(int time, int chan, int key, int velo);
void note_on(int time, int chan, int key, int velo);
void set_tempo(int time, int value, const MIDI_Header *hdr);
void pitch_wheel_change(int time, int chan, int value);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void init_tim2(int n);
void init_tim3(int n);
MIDI_Player* start_game_over_music();
void end_all_music();
void pause_background_music();
void play_explosion();
