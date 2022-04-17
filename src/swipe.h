#ifndef __SWIPE_H
#define __SWIPE_H
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240

typedef struct {
    int x;
    int y;
} Point;

void init_reads();
void init_adc();
void shift_into_vector(Point pt);
int read_y();
int read_x();
int convert_coord(float coord, char dim);

#endif