#include "stm32f0xx.h"
#include <stdio.h>
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
void init_tim3();
void init_reads();
void init_adc();
void read_y();
void read_x();
void nano_wait(unsigned int n);
void init_spi1();
void spi_cmd(unsigned int data);
void spi_data(unsigned int data);
void spi1_display1(const char *string);
void spi1_display2(const char *string);
void spi1_enable_dma();

//-----------------------------------------------
//                 INITIALIZATIONS
//-----------------------------------------------
//Initialize the TIM3 ISR for reading the y coordinate
void init_tim3() {
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

//Enable the DMA unit
void enable_dma(void)
{
    DMA1_Channel5 -> CCR |= DMA_CCR_EN;
}

//-----------------------------------------------
//                TIMERS/ISR
//-----------------------------------------------
void TIM2_IRQHandler()
{

}

//ISR for reading x and y analog coords
void TIM3_IRQHandler() {
    TIM3 -> SR &= ~TIM_SR_UIF;
    //Read x coordinate and output to SPI OLED
    read_x();
    nano_wait(500000);  //500 us between reads
    //Read y coordinate and output to SPI OLED
    read_y();
    nano_wait(500000);  //500 us between reads
}

//-----------------------------------------------
// 2.4 SPI OLED Display
//-----------------------------------------------
void init_spi1() {
    // PA5  SPI1_SCK
    // PA6  SPI1_MISO
    // PA7  SPI1_MOSI
    // PA15 SPI1_NSS
    RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER &= ~GPIO_MODER_MODER15 & ~GPIO_MODER_MODER7 & ~GPIO_MODER_MODER6 & ~GPIO_MODER_MODER5;
    GPIOA -> MODER |= GPIO_MODER_MODER15_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER5_1;
    SPI1 -> CR1 &= ~SPI_CR1_SPE;
    SPI1 -> CR1 |= SPI_CR1_BR;
    SPI1 -> CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI1 -> CR1 |= SPI_CR1_MSTR;
    SPI1 -> CR2 |= SPI_CR2_SSOE;
    SPI1 -> CR2 |= SPI_CR2_NSSP;
    SPI1 -> CR2 |= SPI_CR2_TXDMAEN;
    SPI1 -> CR1 |= SPI_CR1_SPE;

}

void spi_cmd(unsigned int data) {
    while(!(SPI1->SR & SPI_SR_TXE)) {}
    SPI1->DR = data;
}
void spi_data(unsigned int data) {
    spi_cmd(data | 0x200);
}
void spi1_init_oled() {
    nano_wait(1000000);
    spi_cmd(0x38);
    spi_cmd(0x08);
    spi_cmd(0x01);
    nano_wait(2000000);
    spi_cmd(0x06);
    spi_cmd(0x02);
    spi_cmd(0x0c);
}
void spi1_display1(const char *string) {
    spi_cmd(0x02);
    while(*string != '\0') {
        spi_data(*string);
        string++;
    }
}
void spi1_display2(const char *string) {
    spi_cmd(0xc0);
    while(*string != '\0') {
        spi_data(*string);
        string++;
    }
}

void spi1_enable_dma(void) {
    DMA1_Channel3 -> CCR |= DMA_CCR_EN;
}

//-----------------------------------------------
//               TOUCHSCREEN INPUT
//-----------------------------------------------
//Change PB0's MODER to output 3V to Y+
//Change PB1's MODER to read in analog input from X+ (ADC_IN9)
//Drive PB2 low to allow floating values input to TFT pin X-
//Drive PB3 high to set Y- to ground
//Triggered by TIM3 ISR
void read_y() {
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

    for(int x=0; x<10000; x++)
        ;
    float coord = (ADC1 -> DR) * 3 / 4095.0;
    char string[21];
    snprintf(string, 21, "Ycoord: %2.2f", coord);
    spi1_display2(string);
}

//Change PB0's MODER to read in analog input from Y+ (ADC_IN8)
//Change PB1's MODER to output 3V to X+
//Drive PB2 high to set Y- to ground
//Drive PB3 low to allow floating values input to TFT pin X-
//Triggered by TIM3 ISR
void read_x() {
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

    for(int x=0; x<10000; x++)
        ;
    float coord = (ADC1 -> DR) * 3 / 4095.0;
    char string[21];
    snprintf(string, 21, "Xcoord: %2.2f", coord);
    spi1_display1(string);

}

//-----------------------------------------------
//                MISCELLANEOUS
//-----------------------------------------------

int main() {
    init_adc();
    init_reads();
    init_tim3();
    init_spi1();
    spi1_init_oled();
    for(;;)
        ;
}
