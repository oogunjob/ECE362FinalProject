#include <stdlib.h>
#include "stm32f0xx.h"
#include "fruit_ninja.h"

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
extern const Picture blade; // A 3x3 pixelated image to indicate where swipe is

void nano_wait(unsigned int n);


/* ===================================================================================
 * Global Variables
 * ===================================================================================
 */

int playerLives = 3; // number of players lives
int score = 0; // current score
const char fruits_names[NUM_FRUITS] = {'a', 'm', 'g', 'l', 'b'}; // fruits and bomb that will be used for the game
//a == apple, m == melon, g == grape, l == lemon, b == bomb
Fruit *fruits = NULL; // linked list that stores all of the fruits that will be displayed in the game

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
    fruit -> x_speed = (random() % (10 - (-10) + 1)) + (-10); // create a random speed here between -10 and 10
    fruit -> y_speed = (random() % (-60 - (-80) + 1)) + (-80); // create a random speed here -80 and -60
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> next = NULL;
    fruit -> rad = get_fruit_radius(name);

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

/* ===================================================================================
 * Game Logic and Scoring
 * ===================================================================================
 */

void fruit_ninja(){

    // seeds the random number generator
    srandom(TIM15 -> CNT);

    // generates the fruit attributes that will be used for the game
    for (int i = 0; i < NUM_FRUITS; i++){
        generateFruits(fruits_names[i]);
    }

    bool gameOver = false;
    score = 0;
    int fruitCount = 0;

    //Initialize the TIM15 ISR
    init_tim15();
    // continuous loop for the game
    while (!gameOver){
        // TODO: Need some way to check whenever the game is over
        // for event in pygame.event.get():
        //     # checking for closing window
        //     if event.type == pygame.QUIT:
        //         game_running = False

        // make sure that this is the first node in the linked list
        Fruit *current_fruit = fruits;

        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL) {
            if(vector[0].x && vector[0].y)
                update3(vector[0].x, vector[0].x, &blade);
            erase3(vector[1].x, vector[1].y);
            // checks if the fruit should be thrown
            if ((current_fruit -> throw) == true){
                //Disable current interrupt while updating the fruit
                asm("cpsid i");
                current_fruit -> x += current_fruit -> x_speed;       // increases the fruits x coordinate by x_speed
                current_fruit -> y += current_fruit -> y_speed;       // increases the fruits y coordinate by y_speed
                current_fruit -> y_speed += (1 * current_fruit->t); // changes the y-trajectory of the fruit
                current_fruit -> t += 1;                            // changes the trajectory speed for the next iteration

                if ((current_fruit -> x) - (current_fruit -> rad) <= SCREEN_WIDTH){
                    drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                }
                else{
                    // generates a fruit with random attributes
                    generateFruits(current_fruit -> name);
                }

                current_fruit -> hit = isCut(*current_fruit);

                // checks if the player has made contact with a fruit or bomb
                if (current_fruit -> hit){

                    // checks if the object hit was a bomb
                    if (current_fruit -> name == 'b'){

                        // user loses all lives if a bomb is swiped
                        //include dramatic pause
                        playerLives = 0;
                        show_lives(playerLives);
                        //Indicate bomb was cut; display this
                        current_fruit -> image = 'c';
                        drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                        nano_wait(1500000000);  //wait 1.5 seconds, audio effects

                        //break out of loop
                        gameOver = true;
                        break;
                    }

                    // indication that a fruit was swiped
                    else{
                        current_fruit -> image = 'c';
                        drawCurrFruit(current_fruit, current_fruit -> x, current_fruit -> y);
                    }

                    // updates the fruit's speed in the y direction
                    current_fruit -> y_speed += 10;

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
            asm("cpsie i");
            if(gameOver) {break;}
        }
        if(gameOver) {break;}
    }
    show_gameover_screen(score);
}
