#include <stdlib.h>
#include "stm32f0xx.h"
#include "fruit_ninja.h"
#include "swipe.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define NUM_FRUITS 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50
#define BOMB_RADIUS  20 //Diameter of 40

extern const Picture background; // A 240x320 background image

#define NUM_FRUITS 5

// 1 0 to 80
// 2 81 to 160
// 3 161 to 240
// 4 241 to 320
// 5 400 (offscreen)

void nano_wait(unsigned int n);

/* ===================================================================================
 * Global Variables
 * ===================================================================================
 */

int playerLives = 3; // number of players lives
int score = 0; // current score
const char fruits_names[NUM_FRUITS] = {'b', 'a', 'g', 'l', 'm'}; // fruits and bomb that will be used for the game
Fruit *fruits = NULL; // linked list that stores all of the fruits that will be displayed in the game
//a == apple, m == melon, g == grape, l == lemon, b == bomb


/* ===================================================================================
 * ISR
 * ===================================================================================
 */
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

void deleteList(Fruit** head){

    Fruit* current = *head;
    Fruit* next;

    while (current != NULL){
        next = current -> next;
        free(current);
        current = next;
    }

   *head = NULL;
}

/* ===================================================================================
 * Game Logic and Scoring
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
    drawHearts(3);
    showScore(0);
    init_adc();

    //UNCOMMENT FOR SPI DEBUGGING

    //init_spi1();
    //spi1_init_oled();
    init_reads();
    MIDI_Player *mp = midi_init(background_music);
    // The default rate for a MIDI file is 2 beats per second
    // with 48 ticks per beat.  That's 500000/48 microseconds.

    //Initialize the TIM15 ISR
    init_tim15();
    // seeds the random number generator
    srandom(TIM15 -> CNT);
    bool gameOver = false;
    score = 0;

    // generates the fruit attributes that will be used for the game
    for (int i = 0; i < NUM_FRUITS; i++){
        generateFruits(fruits_names[i]);
    }

    // make sure that this is the first node in the linked list
    Fruit *current_fruit = fruits;
    Fruit *head_fruit = fruits;
    // continuous loop for the game
    while (!gameOver){
        //Point current_fruit back to beginning of linked list
        current_fruit = fruits;
        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL) {
            //Loop music player if reaches end
            if (mp->nexttick >= MAXTICKS)
                mp = midi_init(background_music);

            //Read x coordinate and output resulting pixel to SPI OLED
            int x_pixel = read_x();

            //Read y coordinate and output resulting pixel to SPI OLED
            int y_pixel = read_y();

            Point temp = {.x = x_pixel, .y = y_pixel};
            //Shift new point into index 0 of the vector
            //Allow fruit_ninja() to update drawing of this
            shift_into_vector(temp);

            // checks if the fruit should be thrown
            if ((current_fruit -> throw) == true){
                current_fruit -> prev_x = current_fruit -> x;
                current_fruit -> prev_y = current_fruit -> y;
                current_fruit -> x += current_fruit -> x_speed;       // increases the fruits x coordinate by x_speed
                current_fruit -> y += current_fruit -> y_speed;       // increases the fruits y coordinate by y_speed
                current_fruit -> x_speed += (1 * current_fruit->t); // changes the x-trajectory of the fruit
                current_fruit -> t += 1;                            // changes the trajectory speed for the next iteration

                if ((current_fruit -> x) - (current_fruit -> rad) <= SCREEN_WIDTH) {
                    drawCurrFruit(current_fruit, current_fruit -> prev_x, current_fruit -> prev_y);
                }
                else{
                    // generates a fruit with random attributes
                    if(screenIsClear()) {
                        Fruit* ptr = fruits;
                        //Check if any fruit (not bomb) were not hit AND are on screen
                        while(ptr) {
                            if((ptr -> name != 'b') && (ptr -> y < SCREEN_HEIGHT) && !(ptr -> scored))
                                --score;
                            eraseCurrFruit(ptr);
                            ptr = ptr -> next;
                        }
                        if(score < 0) score = 0;
                        showScore(score);
                        for (int i = 0; i < NUM_FRUITS; i++)
                            generateFruits(fruits_names[i]);
                    }
                }

                current_fruit -> hit = isCut(*current_fruit);

                // checks if the player has made contact with a fruit or bomb
                if (current_fruit -> hit){

                    // checks if the object hit was a bomb
                    if (current_fruit -> name == 'b'){

                        // user loses one life if a bomb is swiped
                        if(!(current_fruit -> scored)) {
                            current_fruit -> scored = true;
                            --playerLives;
                            showLives(playerLives);
                            //Indicate bomb was cut; display this
                            current_fruit -> image = 'c';
                            drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                            play_explosion();
                        }
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

                    if (current_fruit -> name != 'b' && !(current_fruit -> scored)) {
                        // updates the score if the fruit swiped was NOT a bomb
                        current_fruit -> scored = true;
                        score += 1;
                        play_score();
                    }
                    showScore(score);

                    // indication that the fruit has been hit already
                    current_fruit -> hit = true;
                }

                // chooses the next fruit to be displayed
                current_fruit = current_fruit -> next;
            }
            if(current_fruit && current_fruit -> x > (SCREEN_WIDTH-40))
                eraseCurrFruit(current_fruit);
            else if(head_fruit -> x > (SCREEN_WIDTH-40))
                eraseCurrFruit(head_fruit);
            if(gameOver) {break;}
        }
        if(gameOver) {break;}
    }
    //Dramatic pause at end. Still display bomb, wipe everything else
    pause_background_music();
    wipe_screen(score, playerLives);
    drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
    nano_wait(750000000);
    MIDI_Player* end = start_game_over_music();
    //Visual effect: blink hearts 6 times
    for(int i = 0; i < 3; i++) {
        drawHearts();
        nano_wait(250000000);
        showLives(0);
        nano_wait(250000000);
    }
    show_gameover_screen(score, playerLives);
    //spin until all ticks played
    while(end->nexttick != MAXTICKS);
    //Turn off music player
    end_all_music();
    deleteList(&fruits);
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
            case 'a':     (fruit -> position) = 4;
                          break;
            default:      (fruit -> position) = 5;
        }
    }
    else{
        // indicates that the fruit does not need to be added back to list after initialization
        new_fruit = false;
    }

    // initialization of the fruit
    fruit -> name = name;
    fruit -> image = 'n'; //'n' == not cut, indicates display function should show not cut image
    fruit -> y = ((random() % (SCREEN_HEIGHT - 1 + 1)) + 1); // create a random y value between 0 and HEIGHT
    fruit -> x = SCREEN_WIDTH; // uses the display width of x
    fruit -> y_speed = (random() % (1 - (-1) + 1)) + (-1); // create a random speed here between -1 and 1
    fruit -> x_speed = (random() % (-15 - (-30) + 1)) + (-30); // create a random speed here -30 and -15
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> rad = get_fruit_radius(name);
    fruit -> prev_y = 0;
    fruit -> prev_x = 0;
    fruit -> scored = false;

    switch(fruit -> position) {
        case 1:     (fruit -> position) = 4;
                    (fruit -> y) = 280;
                    break;
        case 2:     (fruit -> position) = 3;
                    (fruit -> y) = 200;
                      break;
        case 3:     (fruit -> position) = 5;
                    (fruit -> y) = 120;
                    break;
        case 4:     (fruit -> position) = 2;
                    (fruit -> y) = 40;
                    break;
        //Value of 5 = do not show on screen
        default:    (fruit -> position) = 1;
                    (fruit -> y) = 400;
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
