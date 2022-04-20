#include "stm32f0xx.h"
#include "graphics.h"
#include "fruit_ninja.h"

//-----------------------------------------------
//                LCD DISPLAY (SPI2)
//-----------------------------------------------

extern const Picture background; // A 240x320 background image
extern const Picture game_over;  //A 58x80 image displaying "GAME OVER"
extern const Picture black_rectangle; //A 24x20 black rectangle to cover lives
extern const Picture melon; // A 60x60 image of a melon
extern const Picture lemon; // A 50x50 image of a lemon
extern const Picture grape; // A 40x40 image of a grape
extern const Picture apple; // A 50x50 image of an apple
extern const Picture bomb; // A 40x40 image of a bomb
extern const Picture melon_cut; // A 60x60 image of a melon, cut in half
extern const Picture lemon_cut; // A 50x50 image of a lemon, cut in half
extern const Picture grape_cut; // A 40x40 image of a grape, cut in half
extern const Picture apple_cut; // A 50x50 image of an apple, cut in half
extern const Picture bomb_cut; // A 40x40 image of a bomb, exploded
extern const Picture num_0; //A 16x8 image of the number 0
extern const Picture num_1; //A 16x8 image of the number 1
extern const Picture num_2; //A 16x8 image of the number 2
extern const Picture num_3; //A 16x8 image of the number 3
extern const Picture num_4; //A 16x8 image of the number 4
extern const Picture num_5; //A 16x8 image of the number 5
extern const Picture num_6; //A 16x8 image of the number 6
extern const Picture num_7; //A 16x8 image of the number 7
extern const Picture num_8; //A 16x8 image of the number 8
extern const Picture num_9; //A 16x8 image of the number 9


// Copy a subset of a large source picture into a smaller destination.
// sx,sy are the offset into the source picture.
void pic_subset(Picture *dst, const Picture *src, int sx, int sy)
{
    int dw = dst->width;
    int dh = dst->height;
    for(int y=0; y<dh; y++) {
        if (y+sy < 0)
            continue;
        if (y+sy >= src->height)
            break;
        for(int x=0; x<dw; x++) {
            if (x+sx < 0)
                continue;
            if (x+sx >= src->width)
                break;
            dst->pix2[dw * y + x] = src->pix2[src->width * (y+sy) + x + sx];
        }
    }
}

// Overlay a picture onto a destination picture.
// xoffset,yoffset are the offset into the destination picture that the
// source picture is placed.
// Any pixel in the source that is the 'transparent' color will not be
// copied.  This defines a color in the source that can be used as a
// transparent color.
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent)
{
    int count = 0;
    for(int y=0; y<src->height; y++) {
        int dy = y+yoffset;
        if (dy < 0)
            continue;
        if (dy >= dst->height)
            break;
        for(int x=0; x<src->width; x++) {
            int dx = x+xoffset;
            if (dx < 0)
                continue;
            if (dx >= dst->width)
                break;
            unsigned short int p = src->pix2[y*src->width + x];
            if (p != transparent)
                dst->pix2[dy*dst->width + dx] = p;
            count++;
        }
    }
}

// This C macro will create an array of Picture elements.
// Really, you'll just use it as a pointer to a single Picture
// element with an internal pix2[] array large enough to hold
// an image of the specified size.
// BE CAREFUL HOW LARGE OF A PICTURE YOU TRY TO CREATE:
// A 100x100 picture uses 20000 bytes.  You have 32768 bytes of SRAM.
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

//Erase a 3x3 square of pixels at the coordinates specified
void erase3(int x, int y)
{
    TempPicturePtr(tmp,3,3); // Create a temporary 3x3 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Update the displayed position of the img at the coordinates specified
void update3(int x, int y, const Picture* img)
{
    TempPicturePtr(tmp,3,3); // Create a temporary 3x3 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, img, 0xffff); // Overlay the img
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Erase a 40x40 square of pixels at the coordinates specified
void erase40(int x, int y)
{
    TempPicturePtr(tmp,40,40); // Create a temporary 40x40 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Update the displayed position of the img at the coordinates specified
void update40(int x, int y, const Picture* img)
{
    TempPicturePtr(tmp,40,40); // Create a temporary 40x40 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, img, 0x0); // Overlay the img
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Erase a 50x50 square of pixels at the coordinates specified
void erase50(int x, int y)
{
    TempPicturePtr(tmp,50,50); // Create a temporary 50x50 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Update the displayed position of the img at the coordinates specified
void update50(int x, int y, const Picture* img)
{
    TempPicturePtr(tmp,50,50); // Create a temporary 50x50 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, img, 0x0); // Overlay the img
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Erase a 60x60 square of pixels at the coordinates specified
void erase60(int x, int y)
{
    TempPicturePtr(tmp,60,60); // Create a temporary 60x60 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

//Update the displayed position of the img at the coordinates specified, erasing in process
void update60(int x, int y, const Picture* img)
{
    TempPicturePtr(tmp,60,60); // Create a temporary 60x60 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, img, 0x0); // Overlay the img
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Draw
}

void drawCurrFruit(Fruit* fruit, int prev_x, int prev_y) {
    const Picture* img;
    switch(fruit -> name) {
        case 'm':   img = fruit -> image == 'c' ? &melon_cut : &melon;
                    break;
        case 'l':   img = fruit -> image == 'c' ? &lemon_cut : &lemon;
                    break;
        case 'a':   img = fruit -> image == 'c' ? &apple_cut : &apple;
                    break;
        case 'g':   img = fruit -> image == 'c' ? &grape_cut : &grape;
                    break;
        default:    img = fruit -> image == 'c' ? &bomb_cut : &bomb;
    }
    if(fruit -> rad == 20) {
        erase40(prev_x, prev_y);
        update40((fruit -> x), (fruit -> y), img);
    }
    else if(fruit -> rad == 25) {
        erase50(prev_x, prev_y);
        update50((fruit -> x), (fruit -> y), img);
    }
    else {
        erase60(prev_x, prev_y);
        update60((fruit -> x), (fruit -> y), img);
    }
}

void eraseCurrFruit(Fruit* fruit) {
    if(fruit -> rad == 20)
        erase40(fruit -> x, fruit -> y);
    else if(fruit -> rad == 25)
        erase50(fruit -> x, fruit -> y);
    else
        erase60(fruit -> x, fruit -> y);
}

void show_score(int score) {
    //Display 1's digit (16x8 pic)
    switch(score % 10) {
        case 0:     LCD_DrawPicture(6,226,&num_0); break;
        case 1:     LCD_DrawPicture(6,226,&num_1); break;
        case 2:     LCD_DrawPicture(6,226,&num_2); break;
        case 3:     LCD_DrawPicture(6,226,&num_3); break;
        case 4:     LCD_DrawPicture(6,226,&num_4); break;
        case 5:     LCD_DrawPicture(6,226,&num_5); break;
        case 6:     LCD_DrawPicture(6,226,&num_6); break;
        case 7:     LCD_DrawPicture(6,226,&num_7); break;
        case 8:     LCD_DrawPicture(6,226,&num_8); break;
        default:    LCD_DrawPicture(6,226,&num_9);
    }

    //Display 10's digit (16x8 pic)
    switch((score/10) % 10) {
        case 0:     LCD_DrawPicture(6,236,&num_0); break;
        case 1:     LCD_DrawPicture(6,236,&num_1); break;
        case 2:     LCD_DrawPicture(6,236,&num_2); break;
        case 3:     LCD_DrawPicture(6,236,&num_3); break;
        case 4:     LCD_DrawPicture(6,236,&num_4); break;
        case 5:     LCD_DrawPicture(6,236,&num_5); break;
        case 6:     LCD_DrawPicture(6,236,&num_6); break;
        case 7:     LCD_DrawPicture(6,236,&num_7); break;
        case 8:     LCD_DrawPicture(6,236,&num_8); break;
        default:    LCD_DrawPicture(6,236,&num_9);
    }

    //Display 100's digit (16x8 pic)
    switch((score/100) % 10) {
        case 0:     LCD_DrawPicture(6,246,&num_0); break;
        case 1:     LCD_DrawPicture(6,246,&num_1); break;
        case 2:     LCD_DrawPicture(6,246,&num_2); break;
        case 3:     LCD_DrawPicture(6,246,&num_3); break;
        case 4:     LCD_DrawPicture(6,246,&num_4); break;
        case 5:     LCD_DrawPicture(6,246,&num_5); break;
        case 6:     LCD_DrawPicture(6,246,&num_6); break;
        case 7:     LCD_DrawPicture(6,246,&num_7); break;
        case 8:     LCD_DrawPicture(6,246,&num_8); break;
        default:    LCD_DrawPicture(6,246,&num_9);
    }
}

//Remove all objects from screen and show NO lives, but leave score in place
void wipe_screen(int score, int lives) {
    LCD_DrawPicture(0,0,&background);
    show_lives(lives);
    show_score(score);
}

//Function to display GAME OVER across normal background
void show_gameover_screen(int score, int lives) {
    wipe_screen(score, lives);
    TempPicturePtr(tmp,58,80); // Create a temporary 58x80 image.
    pic_subset(tmp, &background, 100, 120); // Copy the background at (100,120)
    pic_overlay(tmp, 0, 0, &game_over, 0x0); // Overlay the img, no offset
    LCD_DrawPicture(100, 120, tmp); // Draw at (100,120)
    for(;;)
        ;
}

//Cover up 3-lives rectangles to display remaining lives
//Offset values into DrawPicture experimentally found
void show_lives(int lives) {
    for(int i = 3 - lives; i > 0; i--) {
        LCD_DrawPicture(4, 10 + (28 * (i - 1)), &black_rectangle);
    }
}

