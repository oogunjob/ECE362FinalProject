#ifndef __OLED_H
#define __OLED_H

void spi_cmd(unsigned int data);
void spi_data(unsigned int data);
void spi1_display1(const char *string);
void spi1_display2(const char *string);
void spi1_enable_dma();
void init_spi1();
void spi1_init_oled();

#endif

