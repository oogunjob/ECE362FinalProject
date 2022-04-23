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

extern Point vector[VECTOR_SIZE];
extern const Picture background; // A 240x320 background image
extern const Picture heart; //A 19x22 heart to indicate lives

#define NUM_FRUITS 5


// 1 0 to 80
// 2 81 to 160
// 3 161 to 240
// 4 241 to 320

void nano_wait(unsigned int n);

/* ===================================================================================
 * Global Variables
 * ===================================================================================
 */

int playerLives = 3; // number of players lives
int score = 0; // current score
const char fruits_names[NUM_FRUITS] = {'a', 'b', 'g', 'l', 'm'}; // fruits and bomb that will be used for the game
Fruit *fruits = NULL; // linked list that stores all of the fruits that will be displayed in the game
//a == apple, m == melon, g == grape, l == lemon, b == bomb


/* ===================================================================================
 * ISR
 * ===================================================================================
 */
//ISR for reading x and y analog coords, storing the Point struct containing
//x and y into a global vector, and using update2() and erase() to animate
//the "blade" and its trajectory
//Note: The tail is best visible/clean when using a stylus instead of finger
void TIM15_IRQHandler() {
    TIM15 -> SR &= ~TIM_SR_UIF;

//DEBUGGING AND VERIFICATION----------------------------------------------------
    //Print this first vector entry to OLED

#define TEST_FIRST
#ifdef TEST_FIRST
    char string[21];
    snprintf(string, 21, "X Pixel: %03d", vector[0].x);
    spi1_display1(string);
    snprintf(string, 21, "Y Pixel: %03d", vector[0].y);
    spi1_display2(string);
#endif

//#define TEST_X
#ifdef TEST_X
    //Print x values of entire vector to OLED
    char string[21];
    snprintf(string, 21, "X%03d %03d %03d %03d %03d", vector[0].x, vector[1].x,
             vector[2].x, vector[3].x, vector[4].x);
    spi1_display1(string);
    snprintf(string, 21, " %03d %03d %03d %03d %03d", vector[5].x, vector[6].x,
             vector[7].x, vector[8].x, vector[9].x);
    spi1_display2(string);
#endif

//#define TEST_Y
#ifdef TEST_Y
    //Print y values of entire vector to OLED

    char string[21];
    snprintf(string, 21, "Y%03d %03d %03d %03d %03d", vector[0].y, vector[1].y,
             vector[2].y, vector[3].y, vector[4].y);
    spi1_display1(string);
    snprintf(string, 21, " %03d %03d %03d %03d %03d", vector[5].y, vector[6].y,
             vector[7].y, vector[8].y, vector[9].y);
    spi1_display2(string);
#endif
    //--------------------------------------------------------------------------
}

//Initialize the TIM15 ISR for reading the x and y coordinates and animating swipes
void init_tim15() {
    RCC -> APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15 -> PSC = 12-1;
    TIM15-> ARR = 400-1;
    TIM15 -> CR1 |= TIM_CR1_CEN;
}

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
bool screenIsClear() {
    Fruit* ptr = fruits;
    while(ptr) {
        if ((ptr -> x) - (ptr -> rad) <= SCREEN_WIDTH)
            return false;
        ptr = ptr -> next;
    }
    return true;
}

void fruit_ninja(){
    LCD_Setup(); // this will call init_lcd_spi()
    //Draw background with upper right corner at (0,0)
    LCD_DrawPicture(0,0,&background);
    for(int i = 3; i > 0; i--) {
        LCD_DrawPicture(4, 10 + (28 * (i - 1)), &heart);
    }
    init_adc();
    init_spi1();
    spi1_init_oled();
    init_reads();

    //Initialize the TIM15 ISR
    init_tim15();
    // seeds the random number generator
    srandom(TIM15 -> CNT);
    bool gameOver = false;
    score = 0;
    int fruitCount = 0;

    // generates the fruit attributes that will be used for the game
    for (int i = 0; i < NUM_FRUITS; i++){
        generateFruits(fruits_names[i]);
    }

    // continuous loop for the game
    while (!gameOver){
        // TODO: Need some way to check whenever the game is over

        // make sure that this is the first node in the linked list
        Fruit *current_fruit = fruits;

        //Read x coordinate and output resulting pixel to SPI OLED
        int x_pixel = read_x();

        //Read y coordinate and output resulting pixel to SPI OLED
        int y_pixel = read_y();

        Point temp = {.x = x_pixel, .y = y_pixel};
        //Shift new point into index 0 of the vector
        //Allow fruit_ninja() to update drawing of this
        shift_into_vector(temp);

#define TEST_FIRST
#ifdef TEST_FIRST
    char string[21];
    snprintf(string, 21, "X Pixel: %03d", vector[0].x);
    spi1_display1(string);
    snprintf(string, 21, "Y Pixel: %03d", vector[0].y);
    spi1_display2(string);
#endif

        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL) {
            // checks if the fruit should be thrown
            if ((current_fruit -> throw) == true){
                current_fruit -> prev_x = current_fruit -> x;
                current_fruit -> prev_y = current_fruit -> y;
                current_fruit -> x += current_fruit -> x_speed;       // increases the fruits x coordinate by x_speed
                current_fruit -> y += current_fruit -> y_speed;       // increases the fruits y coordinate by y_speed

                //DEBUGGING
                /*if(current_fruit -> name == 'm') {
                    char string[21];
                    snprintf(string, 21, "X Pixel: %03d", current_fruit -> x);
                    spi1_display1(string);
                    snprintf(string, 21, "Y Pixel: %03d", current_fruit -> y);
                    spi1_display2(string);
                }*/
                current_fruit -> x_speed += (1 * current_fruit->t); // changes the x-trajectory of the fruit
                current_fruit -> t += 1;                            // changes the trajectory speed for the next iteration

                if ((current_fruit -> x) - (current_fruit -> rad) <= SCREEN_WIDTH) {
                    drawCurrFruit(current_fruit, current_fruit -> prev_x, current_fruit -> prev_y);
                }
                else{
                    // generates a fruit with random attributes
                    eraseCurrFruit(current_fruit);
                    if(screenIsClear()) {
                        for (int i = 0; i < NUM_FRUITS; i++){
                            generateFruits(fruits_names[i]);
                        }
                    }
                }

                current_fruit -> hit = isCut(*current_fruit);

                // checks if the player has made contact with a fruit or bomb
                if (!(current_fruit -> hit) && (vector[0].x > current_fruit -> x) && (vector[0].x < (current_fruit -> x) + 60)
                                            && (vector[0].y > current_fruit -> y) && (vector[0].y < (current_fruit -> y) + 60)){

                    // checks if the object hit was a bomb
                    if (current_fruit -> name == 'b'){

                        // user loses one life if a bomb is swiped
                        --playerLives;
                        show_lives(playerLives);
                        //Indicate bomb was cut; display this
                        current_fruit -> image = 'c';

                        //break out of loop
                        if(!playerLives) {
                            gameOver = true;
                            break;
                        }
                    }

                    // indication that a fruit was swiped
                    else{
                        current_fruit -> image = 'c';
                        drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                    }

                    // updates the fruit's speed in the x direction
                    current_fruit -> x_speed += 1;

                    if (current_fruit -> name != 'b' && !(current_fruit -> hit)) {
                        // updates the score if the fruit swiped was NOT a bomb
                        score += 1;
                    }
                    show_score(score);

                    // indication that the fruit has been hit already
                    current_fruit -> hit = true;
                }

                // chooses the next fruit to be displayed
                current_fruit = current_fruit -> next;
            }
            if(current_fruit)
                eraseCurrFruit(current_fruit);
            if(gameOver) {break;}
        }
        if(gameOver) {break;}
    }
    show_gameover_screen(score, playerLives);
}

/* ===================================================================================
 * Fruit Generation
 * Used for calculating where a certain fruit will be displayed on game board and assigning attributes
 * ===================================================================================
 */

void generateFruits(const char name){
    bool new_fruit = true;               // indication of whether or not a new fruit needs to be added to the list
    Fruit *fruit = search(fruits, name); // searches for fruit in the list

    // checks if the fruit is not already in the list
    if (fruit == NULL){
        // allocates space for the fruit
        fruit = malloc(sizeof(*fruit));
        fruit -> next = NULL;

        // if the fruit is new, initialize which position it will display on screen
        switch(fruit -> name) {
            case 'b':     (fruit -> position) = 1;
                          break;
            case 'l':     (fruit -> position) = 2;
                          break;
            case 'g':     (fruit -> position) = 3;
                          break;
            default:      (fruit -> position) = 4;
        }
    }
    else{
#define DISABLE_ISR asm("cpsid i")
#define ENABLE_ISR asm("cpsie i")
        // indicates that the fruit does not need to be added back to list after initialization
        new_fruit = false;
    }

    // initialization of the fruit
    fruit -> name = name;
    fruit -> image = 'n'; //'n' == not cut, indicates display function should show not cut image
    fruit -> y = ((random() % (SCREEN_HEIGHT - 1 + 1)) + 1); // create a random y value between 0 and HEIGHT
    fruit -> x = SCREEN_WIDTH; // uses the display width of x
    fruit -> y_speed = (random() % (1 - (-1) + 1)) + (-1); // create a random speed here between -1 and 1
    fruit -> x_speed = (random() % (-10 - (-30) + 1)) + (-30); // create a random speed here -30 and -10
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> rad = get_fruit_radius(name);
    fruit -> prev_y = 0;
    fruit -> prev_x = 0;

    switch(fruit -> position) {
        case 1:     (fruit -> position) = 2;
                    (fruit -> y) = 280;
                    break;
        case 2:     (fruit -> position) = 3;
                     (fruit -> y) = 200;
                      break;
        case 3:     (fruit -> position) = 4;
                    (fruit -> y) = 120;
                      break;
        default:     (fruit -> position) = 1;
                      (fruit -> y) = 40;
    }

    // Return the next random floating point number in the range [0.0, 1.0) to keep the fruits inside the gameDisplay
    // This throw key indicates if the fruit will go out-of-bounds
    // If true, update fruit x and y based on speeds. If false, fruit not thrown
    if (random() >= 0.75){
        fruit -> throw = true;
    }
    else{
        fruit -> throw = false;
    }

    // adds the new fruit to the list if it was new
    if (new_fruit == true){
        push(&fruits, fruit);
    }

    return;
}
