#include "music.h"
/* ===================================================================================
 * MIDI
 * ===================================================================================
 */
// The number of simultaneous voices to support.
#define VOICES 15

// We'll use the Timer 6 IRQ to recompute samples and feed those
// samples into the DAC.
void TIM6_DAC_IRQHandler(void)
{
    TIM6 -> SR &= ~TIM_SR_UIF;

    int sample = 0;
    for(int x=0; x < sizeof voice / sizeof voice[0]; x++) {
        if (voice[x].in_use) {
            voice[x].offset += voice[x].step;
            if (voice[x].offset >= N<<16)
                voice[x].offset -= N<<16;
            sample += (wavetable[voice[x].offset>>16] * voice[x].volume) >> 4;
        }
    }
    sample = (sample >> 10) + 2048;
    if (sample > 4095)
        sample = 4095;
    else if (sample < 0)
        sample = 0;
    DAC->DHR12R1 = sample;
}

void TIM7_IRQHandler(void){
    TIM7 -> SR &= ~TIM_SR_UIF;

    int sample = 0;
    for(int x=0; x < sizeof voice / sizeof voice[0]; x++) {
        if (voice[x].in_use) {
            voice[x].offset += voice[x].step;
            if (voice[x].offset >= N<<16)
                voice[x].offset -= N<<16;
            sample += (wavetable[voice[x].offset>>16] * voice[x].volume) >> 4;
        }
    }
    sample = (sample >> 10) + 2048;
    if (sample > 4095)
        sample = 4095;
    else if (sample < 0)
        sample = 0;
    DAC->DHR12R2 = sample;
}


// Initialize the DAC so that it can output analog samples
// on PA4.  Configure DAC1 to be triggered by TIM6 TRGO and DAC2 by TIM7 TRGO.
void init_dac(void)
{
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;
    DAC -> CR &= ~DAC_CR_EN1 & ~DAC_CR_EN2 & ~DAC_CR_TSEL1 & ~DAC_CR_TSEL2;
    DAC -> CR |= DAC_CR_TEN1 | DAC_CR_TEN2 | DAC_CR_TSEL2_1 | DAC_CR_EN1 | DAC_CR_EN2;
}

// Initialize Timer 6 so that it calls TIM6_DAC_IRQHandler
// at exactly RATE times per second.  You'll need to select
// a PSC value and then do some math on the system clock rate
// to determine the value to set for ARR.  Set it to trigger
// the DAC by enabling the Update Trigger in the CR2 MMS field.
void init_tim6(void)
{
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6 -> PSC = 1-1;
    TIM6 -> ARR = (48000000/RATE)-1;
    TIM6 -> DIER |= TIM_DIER_UIE;
    TIM6 -> CR2 = 0x20; //Set MMS bit 1
    TIM6 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM6_DAC_IRQn;
    NVIC_SetPriority(TIM6_DAC_IRQn,0);
}

// Initialize Timer 7 so that it calls TIM7_IRQHandler
// at exactly RATE times per second.  You'll need to select
// a PSC value and then do some math on the system clock rate
// to determine the value to set for ARR.  Set it to trigger
// the DAC by enabling the Update Trigger in the CR2 MMS field.
void init_tim7(void)
{
    RCC -> APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 1-1;
    TIM7 -> ARR = (48000000/RATE)-1;
    TIM7 -> DIER |= TIM_DIER_UIE;
    TIM7 -> CR2 = 0x20; //Set MMS bit 1
    TIM7 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM7_IRQn;
    NVIC_SetPriority(TIM7_IRQn,0);
}

// Find the voice current playing a note, and turn it off.
void note_off(int time, int chan, int key, int velo)
{
    int n;
    for(n=0; n<sizeof voice / sizeof voice[0]; n++) {
        if (voice[n].in_use && voice[n].note == key) {
            voice[n].in_use = 0; // disable it first...
            voice[n].chan = 0;   // ...then clear its values
            voice[n].note = key;
            voice[n].step = step[key];
            return;
        }
    }
}

// Find an unused voice, and use it to play a note.
void note_on(int time, int chan, int key, int velo)
{
    if (velo == 0) {
        note_off(time, chan, key, velo);
        return;
    }
    int n;
    for(n=0; n<sizeof voice / sizeof voice[0]; n++) {
        if (voice[n].in_use == 0) {
            voice[n].note = key;
            voice[n].step = step[key];
            voice[n].offset = 0;
            voice[n].volume = velo;
            voice[n].chan = chan;
            voice[n].in_use = 1;
            return;
        }
    }
}

void set_tempo(int time, int value, const MIDI_Header *hdr)
{
    // This assumes that the TIM2 prescaler divides by 48.
    // It sets the timer to produce an interrupt every N
    // microseconds, where N is the new tempo (value) divided by
    // the number of divisions per beat specified in the MIDI file header.
    TIM2->ARR = value/hdr->divisions - 1;
    TIM3->ARR = value/hdr->divisions - 1;
}

const float pitch_array[] = {
0.943874, 0.945580, 0.947288, 0.948999, 0.950714, 0.952432, 0.954152, 0.955876,
0.957603, 0.959333, 0.961067, 0.962803, 0.964542, 0.966285, 0.968031, 0.969780,
0.971532, 0.973287, 0.975046, 0.976807, 0.978572, 0.980340, 0.982111, 0.983886,
0.985663, 0.987444, 0.989228, 0.991015, 0.992806, 0.994599, 0.996396, 0.998197,
1.000000, 1.001807, 1.003617, 1.005430, 1.007246, 1.009066, 1.010889, 1.012716,
1.014545, 1.016378, 1.018215, 1.020054, 1.021897, 1.023743, 1.025593, 1.027446,
1.029302, 1.031162, 1.033025, 1.034891, 1.036761, 1.038634, 1.040511, 1.042390,
1.044274, 1.046160, 1.048051, 1.049944, 1.051841, 1.053741, 1.055645, 1.057552,
};

void pitch_wheel_change(int time, int chan, int value)
{
    //float multiplier = pow(STEP1, (value - 8192.0) / 8192.0);
    float multiplier = pitch_array[value >> 8];
    for(int n=0; n<sizeof voice / sizeof voice[0]; n++) {
        if (voice[n].in_use && voice[n].chan == chan) {
            voice[n].step = step[voice[n].note] * multiplier;
        }
    }
}

void TIM2_IRQHandler(void)
{
    TIM2 -> SR &= ~TIM_SR_UIF;
    midi_play();
}

void TIM3_IRQHandler(void)
{
    TIM3 -> SR &= ~TIM_SR_UIF;
    midi_play();
}

// Configure timer 2 so that it invokes the Update interrupt
// every n microseconds.  To do so, set the prescaler to divide
// by 48.  Then the CNT will count microseconds up to the ARR value.
// Basically ARR = n-1
// Set the ARPE bit in the CR1 so that the timer waits until the next
// update before changing the effective ARR value.
// Call NVIC_SetPriority() to set a low priority for Timer 2 interrupt.
// See the lab 6 text to understand how to do so.
void init_tim2(int n) {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2 -> PSC = 48-1;
    TIM2 -> ARR = n-1;
    TIM2 -> DIER |= TIM_DIER_UIE;
    TIM2 -> CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM2_IRQn;
    NVIC_SetPriority(TIM2_IRQn,3);
}

void init_tim3(int n) {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3 -> PSC = 48-1;
    TIM3 -> ARR = n-1;
    TIM3 -> DIER |= TIM_DIER_UIE;
    TIM3 -> CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM3_IRQn;
    NVIC_SetPriority(TIM3_IRQn,3);
}

MIDI_Player* start_game_over_music() {
    pause_background_music();
    MIDI_Player *mp = midi_init(game_over_music);
    TIM2 -> CR1 |= TIM_CR1_CEN;
    return mp;
}

void pause_background_music() {
    TIM2 -> CR1 &= ~TIM_CR1_CEN;
    for(int i = 0; i < VOICES; i++) {
        voice[i].in_use = 0;
        voice[i].note = 0;
        voice[i].chan = 0;
        voice[i].volume = 0;
        voice[i].step = 0;
        voice[i].offset = 0;
    }
}

void play_explosion()
{
    MIDI_Player *mp = midi_init(bomb_music);
    TIM3 -> CR1 |= TIM_CR1_CEN;
    TIM6 -> CR1 &= ~TIM_CR1_CEN;
    TIM7 -> CR1 |= TIM_CR1_CEN;
    //Spin until bomb sound done
    while(mp->nexttick != MAXTICKS);
    TIM3 -> CR1 &= ~TIM_CR1_CEN;
    TIM7 -> CR1 &= ~TIM_CR1_CEN;
    TIM6 -> CR1 |= TIM_CR1_CEN;
}

void end_all_music() {
    pause_background_music();
    TIM3 -> CR1 &= ~TIM_CR1_CEN;
}
