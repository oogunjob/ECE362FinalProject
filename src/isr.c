#include "stm32f0xx.h"
#include "isr.h"
#include "graphics.h"
#include "oled.h"
#include "fruit_ninja.h"

#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH  240
#define VECTOR_SIZE 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

extern Point vector[VECTOR_SIZE];
extern const Picture blade; // A 3x3 background image
int meloncut = 0;
int lemoncut = 0;
int grapecut = 0;
int applecut = 0;
int count = 0;
int test_cut_melon(int is_cut);
int test_cut_lemon(int is_cut);
int test_cut_grape(int is_cut);
int test_cut_apple(int is_cut);

#define DISABLE_SWIPE asm("cpsid i")
#define ENABLE_SWIPE asm("cpsie i")
#define NUM_FRUITS 5
int playerLives = 3; // number of players lives
int score = 0; // current score
const char fruits_names[NUM_FRUITS] = {'a', 'b', 'g', 'l', 'm'}; // fruits and bomb that will be used for the game

Fruit *fruits = NULL; // linked list that stores all of the fruits that will be displayed in the game
//a == apple, m == melon, g == grape, l == lemon, b == bomb

int test_cut_melon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+MELON_RADIUS), .y = 240+MELON_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'm', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_lemon(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+LEMON_RADIUS), .y = 180+LEMON_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'l', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_grape(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+GRAPE_RADIUS), .y = 120+GRAPE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'g', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

int test_cut_apple(int is_cut) {
    Point center = {.x = (SCREEN_WIDTH-120+APPLE_RADIUS), .y = 60+APPLE_RADIUS};
    Fruit fr = {.x = center.x, .y = center.y, .name = 'a', .image = 'n'};
    is_cut = isCut(fr);

    if(is_cut == 1) {
        fr.image = 'c';
        count++;
    }

    drawCurrFruit(&fr, fr.x, fr.y);
    return is_cut;
}

//ISR for reading x and y analog coords, storing the Point struct containing
//x and y into a global vector, and using update2() and erase() to animate
//the "blade" and its trajectory
//Note: The tail is best visible/clean when using a stylus instead of finger
void TIM15_IRQHandler() {
    TIM15 -> SR &= ~TIM_SR_UIF;


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

        //Disable current interrupt while updating the fruit
        DISABLE_SWIPE;

        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL) {
            if(vector[0].x && vector[0].y)
                update3(vector[0].x, vector[0].x, &blade);

            erase3(vector[1].x, vector[1].y);
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
                    generateFruits(current_fruit -> name);
                }

                current_fruit -> hit = isCut(*current_fruit);

                // checks if the player has made contact with a fruit or bomb
                if (!(current_fruit -> hit) && (vector[0].x > current_fruit -> x) && (vector[0].x < (current_fruit -> x) + 60)
                                            && (vector[0].y > current_fruit -> y) && (vector[0].y < (current_fruit -> y) + 60)){

                    // checks if the object hit was a bomb
                    if (current_fruit -> name == 'b'){

                        // user loses all lives if a bomb is swiped
                        //include dramatic pause
                        playerLives = 0;
                        show_lives(playerLives);
                        //Indicate bomb was cut; display this
                        current_fruit -> image = 'c';

                        //break out of loop
                        gameOver = true;
                        break;
                    }

                    // indication that a fruit was swiped
                    else{
                        current_fruit -> image = 'c';
                        drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                    }

                    // updates the fruit's speed in the x direction
                    current_fruit -> x_speed += 1;

                    if (current_fruit -> name != 'b') {
                        // updates the score if the fruit swiped was NOT a bomb
                        score += 1;
                    }
                    show_score(score);

                    // indication that the fruit has been hit already
                    current_fruit -> hit = true;
                }
                else if(fruitCount < 2){
                    // generates new fruit
                    generateFruits(current_fruit -> name);
                    fruitCount++;
                }

                // chooses the next fruit to be displayed
                current_fruit = current_fruit -> next;
            }
            //Re-enable interrupt after updating fruit
            //ENABLE_SWIPE;
            eraseCurrFruit(current_fruit);
            if(gameOver) {break;}
        }
        //wipe_screen(score, playerLives);
        if(gameOver) {break;}
    }
    show_gameover_screen(score);

//DEBUGGING AND VERIFICATION----------------------------------------------------
    //Print this first vector entry to OLED

//#define TEST_FIRST
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
    TIM15 -> PSC = 120-1;
    TIM15-> ARR = 400-1;
    TIM15 -> DIER |= TIM_DIER_UIE;
    TIM15 -> CR1 |= TIM_CR1_CEN;
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
    }
    else{
        // indicates that the fruit does not need to be added back to list after initialization
        new_fruit = false;
    }

    // initialization of the fruit
    fruit -> name = name;
    fruit -> image = 'n'; //'n' == not cut, indicates display function should show not cut image
    fruit -> y = (random() % (SCREEN_HEIGHT - 100 + 1)) + 100; // create a random y value between 100 and HEIGHT
    fruit -> x = SCREEN_WIDTH; // uses the display width of x
    fruit -> y_speed = (random() % (10 - (-10) + 1)) + (-10); // create a random speed here between -10 and 10
    fruit -> x_speed = (random() % (-10 - (-30) + 1)) + (-30); // create a random speed here -30 and -10
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> rad = get_fruit_radius(name);
    fruit -> prev_y = 0;
    fruit -> prev_x = 0;

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
