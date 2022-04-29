#include "stm32f0xx.h"
#include <stdio.h>
#include "swipe.h"
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 3
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50

//A vector to be externally available to other functions
//Holds VECTOR_SIZE entries of the most recent points
Point vector[VECTOR_SIZE];

//-----------------------------------------------
//               INITIALIZATIONS
//-----------------------------------------------

//Initialize PB0-3 for use in reading touch-screen coordinate input
void init_reads() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER2 |
                        GPIO_MODER_MODER1 | GPIO_MODER_MODER0);
    GPIOB -> MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER2_0 |
                      GPIO_MODER_MODER1_0 | GPIO_MODER_MODER0_0;
}

//Initialize the ADC unit
void init_adc()
{
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER |= GPIO_MODER_MODER1 | GPIO_MODER_MODER0;

    RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC -> CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC -> CR2 & RCC_CR2_HSI14RDY));
    ADC1 -> CR |= ADC_CR_ADEN;
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));
    ADC1 -> CHSELR = 0;
    ADC1 -> CHSELR |= 1 << 1;
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));
}


//-----------------------------------------------
//               TOUCHSCREEN INPUT
//-----------------------------------------------

//Change PB0's MODER to read in analog input from Y+ (ADC_IN8)
//Change PB1's MODER to output 3V to X+
//Drive PB2 high to set X- to ground
//Drive PB3 low to allow floating values input to TFT pin Y-
//Triggered by TIM3 ISR
int read_x() {
    GPIOB -> MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
    GPIOB -> MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1_0;
    GPIOB -> ODR |= GPIO_ODR_1;
    GPIOB -> ODR |= GPIO_ODR_2;
    GPIOB -> ODR &= ~GPIO_ODR_3;
    TIM2 -> SR &= ~TIM_SR_UIF;
    ADC1 -> CHSELR = 0;
    ADC1 -> CHSELR = (1<<8);
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));
    ADC1 -> CR |= ADC_CR_ADSTART;
    while(!(ADC1 -> ISR & ADC_ISR_EOC));

    //Wait for ADC operations to settle
    for(int x=0; x<10000; x++)
        ;
    float coord = (ADC1 -> DR) * 2.83 / 4095.0;
    int pixel = convert_coord(coord, 'x');
    return pixel;
}

//Change PB0's MODER to output 3V to Y+
//Change PB1's MODER to read in analog input from X+ (ADC_IN9)
//Drive PB2 low to allow floating values input to TFT pin X-
//Drive PB3 high to set Y- to ground
//Triggered by TIM3 ISR
int read_y() {
    GPIOB -> MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
    GPIOB -> MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1;
    GPIOB -> ODR |= GPIO_ODR_0;
    GPIOB -> ODR &= ~GPIO_ODR_2;
    GPIOB -> ODR |= GPIO_ODR_3;
    TIM2 -> SR &= ~TIM_SR_UIF;
    ADC1 -> CHSELR = 0;
    ADC1 -> CHSELR = (1<<9);
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));
    ADC1 -> CR |= ADC_CR_ADSTART;
    while(!(ADC1 -> ISR & ADC_ISR_EOC));

    //Wait for ADC operations to settle
    for(int x=0; x<10000; x++)
        ;
    float coord = (ADC1 -> DR) * 2.83 / 4095.0;
    int pixel = convert_coord(coord, 'y');
    return pixel;
}

//-----------------------------------------------
//    TOUCHSCREEN INPUT INTERPRETATION
//-----------------------------------------------

//Convert given coordinate in voltage to representative pixel
int convert_coord(float coord, char dim) {
    //Convert value and keep in hundreds
    //Ex. 1.732 -> 173, 2.467 -> 247
    int value = (int)(coord * 100 + 0.5);

    //Dead zone (no pixels) until 0.23 V reached
    if(dim == 'y')
        value -= 23;
    //Dead zone (no pixels) until 0.19 V reached
    else
        value -= 19;

    //If, after subtracting, value is 0 or negative, return 0
    //Also, if value exceeds max allowable, return 0 (went off screen)
    if(dim == 'x' && (value <= 0 || value > 235))
        return 0;
    else if(dim == 'y' && (value <= 0 || value > 227))
            return 0;

//Ratio of max voltage ~2.27 to pixel amount:
//X: 240:227 == 1.057 ~= 1.08 <-- (trial and error value)
//Y: 320:227 == 1.410 <-- (trial and error value)

    //Return, having accounted for voltage off screen
    //Setup coordinate plane such that origin is at upper right corner of screen
    if(dim == 'y')
        return 1.41 * value;
    else
        return 1.08 * value;
}

//Shift Point object into front of vector buffer for continual update of point location
void shift_into_vector(Point pt) {
    for(int i = VECTOR_SIZE - 1; i > 0; i--) {
        vector[i] = vector[i - 1];
    }
    vector[0] = pt;
}

int isCut(Fruit fr) {
    int radius = 0;
    int distSwipeX = 0;
    int distSwipeY = 0;

    switch(fr.name) {
        case 'm' : radius = MELON_RADIUS;
                            break;
        case 'l' : radius = LEMON_RADIUS;
                            break;
        case 'g' : radius = GRAPE_RADIUS;
                            break;
        default  : radius = APPLE_RADIUS;
    }
    int distRad = radius*radius;
    distSwipeX = (fr.x - vector[0].x) * (fr.x - vector[0].x);
    distSwipeY = (fr.y - vector[0].y) * (fr.y - vector[0].y);

    //Invalid swipe if vector[1] is 0 (swipe must have filled at least vector[1:0])
    if(vector[1].x == 0 || vector[1].y == 0) {
        return 0;
    }

    return distSwipeX < distRad && distSwipeY < distRad ? 1 : 0;
}
