#include <stdlib.h>
#include "stm32f0xx.h"
#include "fruit_ninja.h"
#include "oled.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define VECTOR_SIZE 15
#define FPS 12 // how often the game display refreshes // TODO: May not need this
#define NUM_FRUITS 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define BOMB_RADIUS  20 //Diameter of 40
#define DISABLE_SWIPE asm("cpsid i")
#define ENABLE_SWIPE asm("cpsie i")

extern Point vector[VECTOR_SIZE];
extern const Picture blade; // A 3x3 pixelated image to indicate where swipe is
extern const Picture background; // A 240x320 background image

void nano_wait(unsigned int n);


/* ===================================================================================
 * Global Variables
 * ===================================================================================
 */

//int playerLives = 3; // number of players lives
//int score = 0; // current score
//const char fruits_names[NUM_FRUITS] = {'a', 'm', 'g', 'l', 'b'}; // fruits and bomb that will be used for the game
//a == apple, m == melon, g == grape, l == lemon, b == bomb
//Fruit *fruits = NULL; // linked list that stores all of the fruits that will be displayed in the game

/* ===================================================================================
 * Linked List Functions
 * ===================================================================================
 */

void push(Fruit **head, Fruit *fruit){

    /* link the old list off the new node */
    fruit -> next = (*head);

    /* move the head to point to the new node */
    (*head) = fruit;
}

Fruit *search(Fruit *head, const char name){
    Fruit *current = head; // Initialize current node

    // searches through linked list to see if the fruit with same name already exists
    while (current != NULL){
        // if the fruit is already in the linked list, return the node
        if (current -> name == name)
            return current;

        current = current->next;
    }
    return NULL;
}

/* ===================================================================================
 * Display Functions
 * Used for updating the display of the TFT LCD such as changing the score, adding new fruits, etc
 * ===================================================================================
 */

int get_fruit_radius(char name) {
    switch(name) {
       case 'm':   return MELON_RADIUS;
       case 'l':   return LEMON_RADIUS;
       case 'g':   return GRAPE_RADIUS;
       case 'a':   return APPLE_RADIUS;
       default :   return BOMB_RADIUS;
    }
}



/* ===================================================================================
 * Game Logic and Scoring
 * ===================================================================================
 */

void fruit_ninja(){
    LCD_Setup(); // this will call init_lcd_spi()
    //Draw background with upper right corner at (0,0)
    LCD_DrawPicture(0,0,&background);
    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();



    //Initialize the TIM15 ISR
    init_tim15();
    // seeds the random number generator
    srandom(TIM15 -> CNT);
    NVIC -> ISER[0] = 1<<TIM15_IRQn;
    for(;;) {
        //Read x coordinate and output resulting pixel to SPI OLED
        int x_pixel = read_x();

        //Read y coordinate and output resulting pixel to SPI OLED
        int y_pixel = read_y();

        Point temp = {.x = x_pixel, .y = y_pixel};
        //If point is a valid swipe location
        if(temp.x == 0 || temp.y == 0) {
            DISABLE_SWIPE;
            ENABLE_SWIPE;
        }
        //Shift new point into index 0 of the vector
        //Allow fruit_ninja() to update drawing of this
        shift_into_vector(temp);
    }

}
