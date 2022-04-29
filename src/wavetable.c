#include <math.h>
#include "midiplay.h"

short int wavetable[N];

// 3/4 amplitude sine wave added to 1/4 amplitude sawtooth wave
void init_wavetable_hybrid2(void) {
    int x;
    for(x=0; x<N; x++)
        wavetable[x] = 3*8191 * sin(2 * M_PI * x / N) + 8191.0 * (x - N/2) / (1.0*N);
}
