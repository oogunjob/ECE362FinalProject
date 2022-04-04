#include "stm32f0xx.h"
//NOTES:
//TIM6 for DAC
//TIM2 for ADC
//TIM3 for read_x() and read_y()

//TO-DO:
//--Modify code such that read_x() and read_y() can coexist
//  (currently, only one signal shifts on scope and the other signal does
//   not behave as expected, depending on which fn is called first)
//Setup DAC
//

//-----------------------------------------------
//               FUNCTION DECLARATIONS
//-----------------------------------------------
void read_y();
void read_x();
void nano_wait(unsigned int n);

//-----------------------------------------------
//                 INITIALIZATIONS
//-----------------------------------------------
//Initialize the TIM3 ISR for reading the y coordinate
void init_tim3(void) {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3 -> PSC = 120-1;
    TIM3 -> ARR = 400-1;
    TIM3 -> DIER |= TIM_DIER_UIE;
    TIM3 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] |= 1<<TIM3_IRQn;
}

//Initialize PB0-3 for use in reading touch-screen coordinate input
void init_reads() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER2 |
                        GPIO_MODER_MODER1 | GPIO_MODER_MODER0);
    GPIOB -> MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER2_0 |
                      GPIO_MODER_MODER1_0 | GPIO_MODER_MODER0_0;
}

//-----------------------------------------------
//                TIMERS/ISR
//-----------------------------------------------
void TIM7_IRQHandler() {
    TIM7 -> SR &= ~TIM_SR_UIF;

}

void TIM3_IRQHandler() {
    TIM3 -> SR &= ~TIM_SR_UIF;
    read_x();
    nano_wait(100000);
    read_y();
    nano_wait(100000);
}


//-----------------------------------------------
//               TOUCHSCREEN INPUT
//-----------------------------------------------
//Drive PB3 and PB0 low to allow floating values input to TFT pins X- and X+
//Drive PB1 and PB2 high to set Y- to ground and Y+ to 3V
//Triggered by TIM3 ISR
void read_y() {
    GPIOB -> ODR &= ~GPIO_ODR_0;
    GPIOB -> ODR &= ~GPIO_ODR_3;
    GPIOB -> ODR |= GPIO_ODR_1;
    GPIOB -> ODR |= GPIO_ODR_2;
}

//Drive PB3 and PB0 high to set X- to ground and X+ to 3V
//Drive PB1 and PB2 low to allow floating values input to TFT pins Y- and Y+
//Triggered by TIM7 ISR
void read_x() {
    GPIOB -> ODR |= GPIO_ODR_0;
    GPIOB -> ODR |= GPIO_ODR_3;
    GPIOB -> ODR &= ~GPIO_ODR_1;
    GPIOB -> ODR &= ~GPIO_ODR_2;
}

//-----------------------------------------------
//                MISCELLANEOUS
//-----------------------------------------------
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

int main(void) {
    init_reads();
    init_tim3();
    for(;;)
        ;
}
