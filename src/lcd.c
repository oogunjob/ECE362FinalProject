
//============================================================================
// lcd.c: Adapted from the lcdwiki.com examples.
//============================================================================

#include "stm32f0xx.h"
#include <stdint.h>
#include "lcd.h"

lcd_dev_t lcddev;

#define SPI SPI2

#define CS_NUM  8
#define CS_BIT  (1<<CS_NUM)
#define CS_HIGH do { GPIOB->BSRR = GPIO_BSRR_BS_8; } while(0)
#define CS_LOW do { GPIOB->BSRR = GPIO_BSRR_BR_8; } while(0)
#define RESET_NUM 11
#define RESET_BIT (1<<RESET_NUM)
#define RESET_HIGH do { GPIOB->BSRR = GPIO_BSRR_BS_11; } while(0)
#define RESET_LOW  do { GPIOB->BSRR = GPIO_BSRR_BR_11; } while(0)
#define DC_NUM 14
#define DC_BIT (1<<DC_NUM)
#define DC_HIGH do { GPIOB->BSRR = GPIO_BSRR_BS_14; } while(0)
#define DC_LOW  do { GPIOB->BSRR = GPIO_BSRR_BR_14; } while(0)

void init_lcd_spi(void)
{
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= ~(GPIO_MODER_MODER15 | GPIO_MODER_MODER14 | GPIO_MODER_MODER13 | GPIO_MODER_MODER11 | GPIO_MODER_MODER8);
    GPIOB -> MODER |= GPIO_MODER_MODER14_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER8_0;
    GPIOB -> MODER |= GPIO_MODER_MODER15_1 | GPIO_MODER_MODER13_1;
    GPIOB -> AFR[1] &= ~((0xf << 4*(15-8)) | (0xf << 4*(13-8)));
    GPIOB -> ODR |= GPIO_ODR_14 | GPIO_ODR_11 | GPIO_ODR_8;

    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 &= ~SPI_CR1_BR;
    SPI2 -> CR2 = SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
    SPI2 -> CR1 |= SPI_CR1_MSTR;
    SPI2 -> CR1 |= SPI_CR1_SSM;
    SPI2 -> CR1 |= SPI_CR1_SSI;
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

// Set the CS pin low if val is non-zero.
// Note that when CS is being set high again, wait on SPI to not be busy.
static void tft_select(int val)
{
    if (val == 0) {
        while(SPI1->SR & SPI_SR_BSY)
            ;
        CS_HIGH;
    } else {
        while((GPIOB->ODR & (CS_BIT)) == 0) {
            ; // If CS is already low, this is an error.  Loop forever.
            // This has happened because something called a drawing subroutine
            // while one was already in process.  For instance, the main()
            // subroutine could call a long-running LCD_DrawABC function,
            // and an ISR interrupts it and calls another LCD_DrawXYZ function.
            // This is a common mistake made by students.
            // This is what catches the problem early.
        }
        CS_LOW;
    }
}

// If val is non-zero, set nRESET low to reset the display.
static void tft_reset(int val)
{
    if (val) {
        RESET_LOW;
    } else {
        RESET_HIGH;
    }
}

// If
static void tft_reg_select(int val)
{
    if (val == 1) { // select registers
        DC_LOW; // clear
    } else { // select data
        DC_HIGH; // set
    }
}

//============================================================================
// Wait for n nanoseconds. (Maximum: 4.294 seconds)
//============================================================================
static inline void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void LCD_Reset(void)
{
    lcddev.reset(1);      // Assert reset
    nano_wait(100000000); // Wait
    lcddev.reset(0);      // De-assert reset
    nano_wait(50000000);  // Wait
}

// Write to an LCD "register"
void LCD_WR_REG(uint8_t data)
{
    while((SPI->SR & SPI_SR_BSY) != 0)
        ;
    // Don't clear RS until the previous operation is done.
    lcddev.reg_select(1);
    *((uint8_t*)&SPI->DR) = data;
}

// Write 8-bit data to the LCD
void LCD_WR_DATA(uint8_t data)
{
    while((SPI->SR & SPI_SR_BSY) != 0)
        ;
    // Don't set RS until the previous operation is done.
    lcddev.reg_select(0);
    *((uint8_t*)&SPI->DR) = data;
}

// Prepare to write 16-bit data to the LCD
void LCD_WriteData16_Prepare()
{
    lcddev.reg_select(0);
    SPI->CR2 |= SPI_CR2_DS;
}

// Write 16-bit data
void LCD_WriteData16(u16 data)
{
    while((SPI->SR & SPI_SR_TXE) == 0)
        ;
    SPI->DR = data;
}

// Finish writing 16-bit data
void LCD_WriteData16_End()
{
    SPI->CR2 &= ~SPI_CR2_DS; // bad value forces it back to 8-bit mode
}

// Select an LCD "register" and write 8-bit data to it.
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
    LCD_WR_REG(LCD_Reg);
    LCD_WR_DATA(LCD_RegValue);
}

// Issue the "write RAM" command configured for the display.
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
}

// Configure the lcddev fields for the display orientation.
void LCD_direction(u8 direction)
{
    lcddev.setxcmd=0x2A;
    lcddev.setycmd=0x2B;
    lcddev.wramcmd=0x2C;
    switch(direction){
    case 0:
        lcddev.width=LCD_W;
        lcddev.height=LCD_H;
        LCD_WriteReg(0x36,(1<<3)|(0<<6)|(0<<7));//BGR==1,MY==0,MX==0,MV==0
        break;
    case 1:
        lcddev.width=LCD_H;
        lcddev.height=LCD_W;
        LCD_WriteReg(0x36,(1<<3)|(0<<7)|(1<<6)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
        break;
    case 2:
        lcddev.width=LCD_W;
        lcddev.height=LCD_H;
        LCD_WriteReg(0x36,(1<<3)|(1<<6)|(1<<7));//BGR==1,MY==0,MX==0,MV==0
        break;
    case 3:
        lcddev.width=LCD_H;
        lcddev.height=LCD_W;
        LCD_WriteReg(0x36,(1<<3)|(1<<7)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
        break;
    default:break;
    }
}

// Do the initialization sequence for the display.
void LCD_Init(void (*reset)(int), void (*select)(int), void (*reg_select)(int))
{
    lcddev.reset = tft_reset;
    lcddev.select = tft_select;
    lcddev.reg_select = tft_reg_select;
    if (reset)
        lcddev.reset = reset;
    if (select)
        lcddev.select = select;
    if (reg_select)
        lcddev.reg_select = reg_select;
    lcddev.select(1);
    LCD_Reset();
    // Initialization sequence for 2.2inch ILI9341
    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD9); // C1
    LCD_WR_DATA(0X30);
    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0X12);
    LCD_WR_DATA(0X81);
    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x7A);
    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xC0);    // Power control
    LCD_WR_DATA(0x21);   // VRH[5:0]  //1B
    LCD_WR_REG(0xC1);    // Power control
    LCD_WR_DATA(0x12);   // SAP[2:0];BT[3:0] //01
    LCD_WR_REG(0xC5);    // VCM control
    LCD_WR_DATA(0x39);   // 3F
    LCD_WR_DATA(0x37);   // 3C
    LCD_WR_REG(0xC7);    // VCM control2
    LCD_WR_DATA(0XAB);   // B0
    LCD_WR_REG(0x36);    // Memory Access Control
    LCD_WR_DATA(0x48);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1B);   // 1A
    LCD_WR_REG(0xB6);    // Display Function Control
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0xA2);
    LCD_WR_REG(0xF2);    // 3Gamma Function Disable
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26);    // Gamma curve selected
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xE0);     // Set Gamma
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x1F);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x4B);
    LCD_WR_DATA(0XA8);
    LCD_WR_DATA(0x3B);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0XE1);      // Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1C);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x47);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x3f);
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xef);
    LCD_WR_REG(0x11);     // Exit Sleep
    nano_wait(120000000); // Wait 120 ms
    LCD_WR_REG(0x29);     // Display on

    LCD_direction(USE_HORIZONTAL);
    lcddev.select(0);
}

void LCD_Setup() {
    init_lcd_spi();
    tft_select(0);
    tft_reset(0);
    tft_reg_select(0);
    LCD_Init(tft_reset, tft_select, tft_reg_select);
}

//===========================================================================
// Select a subset of the display to work on, and issue the "Write RAM"
// command to prepare to send pixel data to it.
//===========================================================================
void LCD_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(xStart>>8);
    LCD_WR_DATA(0x00FF&xStart);
    LCD_WR_DATA(xEnd>>8);
    LCD_WR_DATA(0x00FF&xEnd);

    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(yStart>>8);
    LCD_WR_DATA(0x00FF&yStart);
    LCD_WR_DATA(yEnd>>8);
    LCD_WR_DATA(0x00FF&yEnd);

    LCD_WriteRAM_Prepare();
}

//===========================================================================
// Draw a picture with upper left corner at (x0,y0).
//===========================================================================
void LCD_DrawPicture(int x0, int y0, const Picture *pic)
{
    int x1 = x0 + pic->width-1;
    int y1 = y0 + pic->height-1;
    if (x0 >= lcddev.width || y0 >= lcddev.height || x1 < 0 || y1 < 0)
        return; // Completely outside of screen.  Nothing to do.
    lcddev.select(1);
    int xs=0;
    int ys=0;
    int xe=pic->width;
    int ye=pic->height;
    if (x0 < 0) {
        xs = -x0;
        x0 = 0;
    }
    if (y0 < 0) {
        ys = -y0;
        y0 = 0;
    }
    if (x1 >= lcddev.width) {
        xe -= x1 - (lcddev.width - 1);
        x1 = lcddev.width - 1;
    }
    if (y1 >= lcddev.height) {
        ye -= y1 - (lcddev.height - 1);
        y1 = lcddev.height - 1;
    }

    LCD_SetWindow(x0,y0,x1,y1);
    LCD_WriteData16_Prepare();

    u16 *data = (u16 *)pic->pixel_data;
    for(int y=ys; y<ye; y++) {
        u16 *row = &data[y * pic->width + xs];
        for(int x=xs; x<xe; x++)
            LCD_WriteData16(*row++);
    }

    LCD_WriteData16_End();
    lcddev.select(0);
}
