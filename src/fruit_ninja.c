#include <stdlib.h>
#include <time.h>

#include "stm32f0xx.h"
#include "fruit_ninja.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FPS 12 // how often the game display refreshes // TODO: May not need this
#define NUM_FRUITS 5
#define MELON_RADIUS 30 //Diameter of 60
#define LEMON_RADIUS 25 //Diameter of 50
#define GRAPE_RADIUS 20 //Diameter of 40
#define APPLE_RADIUS 25 //Diameter of 50

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

    //TODO: Need to add them here

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
    fruit -> y = (rand() % (SCREEN_HEIGHT - 100 + 1)) + 100; // create a random y value between 100 and HEIGHT
    fruit -> x = SCREEN_WIDTH; // uses the display width of x
    fruit -> x_speed = (rand() % (10 - (-10) + 1)) + (-10); // create a random speed here between -10 and 10
    fruit -> y_speed = (rand() % (-60 - (-80) + 1)) + (-80); // create a random speed here -80 and -60
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> next = NULL;

    // Return the next random floating point number in the range [0.0, 1.0) to keep the fruits inside the gameDisplay
    // This throw key indicates if the fruit will go out-of-bounds
    // If true, update fruit x and y based on speeds. If false, generate new fruit
    if (rand() >= 0.75){
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

int isCut(Fruit fr, Point swipe) {
    int radius = 0;

    switch(fr.name) {
        case 'm' : radius = MELON_RADIUS;
                            break;
        case 'l' : radius = LEMON_RADIUS;
                            break;
        case 'g' : radius = GRAPE_RADIUS;
                            break;
        default  : radius = APPLE_RADIUS;
    }

    int distSwipeX = (fr.x - swipe.x) * (fr.x - swipe.x);
    int distSwipeY = (fr.y - swipe.y) * (fr.y - swipe.y);
    int distRad = radius*radius;

    return distSwipeX < distRad ? distSwipeY < distRad ? 1 : 0 : 0;
}

void fruit_ninja(){

    // seeds the random number generator
    srand(time(NULL));

    // generates the fruit attributes that will be used for the game
    for (int i = 0; i < NUM_FRUITS; i++){
        generateFruits(fruits_names[i]);
    }

    bool firstRound = true;
    bool gameOver = true;
    bool GameRunning = true;

    //Initialize the TIM3 ISR
    init_tim3();
    // continuous loop for the game
    while (GameRunning){

        // checks if the game is over
        if (gameOver){
            // if the game is still in the first round, display the game over screen and update the round
            if (firstRound){
                show_gameover_screen();
                firstRound = false;
            }

            gameOver = false;
            playerLives = 3;

            // TODO: Need to change this display to draw the lives for us
            // Use what Bailey said and change it to draw a black box over the image lives

            score = 0; // updates the score back to 0
        }

        // TODO: Need some way to check whenever the game is over
        // for event in pygame.event.get():
        //     # checking for closing window
        //     if event.type == pygame.QUIT:
        //         game_running = False

        // make sure that this is the first node in the linked list
        Fruit *current_fruit = fruits;

        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL){

            // checks if the fruit should be thrown
            if ((current_fruit -> throw) == true){
                current_fruit -> x += current_fruit -> x_speed;       // increases the fruits x coordinate by x_speed
                current_fruit -> y += current_fruit -> y_speed;       // increases the fruits y coorindate by y_speed
                current_fruit -> y_speed += (1 * current_fruit->t); // changes the y-trajectory of the fruit
                current_fruit -> t += 1;                            // changes the trajectory speed for the next iteration

                if (current_fruit -> x <= SCREEN_WIDTH){
                    // TODO: Display the fruit
                    // gameDisplay.blit(value['img'], (value['x'], value['y']))    #displaying the fruit inside screen dynamically
                    printf("\n"); // place holder
                }
                else{
                    // generates a fruit with random attributes
                    generateFruits(current_fruit -> name);
                }

                // TODO: Include the X and Y position of the fruit
                // This should be a vector, ask Bailey
                // current_position = pygame.mouse.get_pos()
                // current_position[0] == X
                // current_position[1] == Y
                int User_X;
                int User_Y;

                // checks if the player has made contact with a fruit or bomb
                if (!(current_fruit -> hit) && (User_X > current_fruit -> x) && (User_X < (current_fruit -> x) + 60) && (User_Y > (current_fruit -> y)) && (User_Y < (current_fruit -> y) + 60)){

                    // checks if the object hit was a bomb
                    if (current_fruit -> name == 'b'){

                        // user loses all lives if a bomb is swiped
                        //TODO: Implement hideLives
                        playerLives -= 1;
                        showLives(playerLives);

                        // if the user swipes a bomb, GAME OVER message should be displayed and the window should be reset
                        if (playerLives == 0){
                            show_gameover_screen();
                            gameOver = true;
                        }

                        // updates the new image for the bomb after explosion
                        // TODO: Change this location
                    }

                    // indication that a fruit was swiped
                    else{
                        // TODO Need to change this to open whichever image it was when the fruit gets cut
                        // updates a new image for the fruit after explosion

                    }

                    // updates the fruit's new image after being swiped along with the speed in the x direction
                    current_fruit -> image = 'c';  //'c' == cut, indicates display function should show cut fruit
                    current_fruit -> x_speed += 10;

                    if (current_fruit -> name != 'b'){
                        // updates the score if the fruit swiped was NOT a bomb
                        score += 1;
                    }

                    // TODO: Update the score here
                    //  UpdateScore();

                    // indication that the fruit has been hit already
                    current_fruit -> hit = true;
                }
                else{
                    // generates new fruit
                    generateFruits(current_fruit -> name);
                }

                // chooses the next fruit to be displayed
                current_fruit = current_fruit -> next;
            }
        }
    }
}
