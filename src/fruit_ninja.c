#include <stdlib.h>
#include <time.h>

#include "stm32f0xx.h"
#include "fruit_ninja.h"

#define WIDTH 320
#define HEIGHT 240
#define FPS 12 // how often the game display refreshes // TODO: May not need this
#define NUM_FRUITS 5

/* ===================================================================================
 * Global Variables
 * ===================================================================================
 */

int playerLives = 3; // number of players lives
int score = 0; // current score
const char * fruits_names[NUM_FRUITS] = {"apple", "watermelon", "orange", "banana", "bomb"}; // fruits and bomb that will be used for the game
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

Fruit *search(Fruit *head, const char *name){
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

void generateFruits(const char *name){
    bool new_fruit = true;               // indication of wheter or not a new fruit needs to be added to the list
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

    // initalization of the fruit
    fruit -> name = name;
    fruit -> image = "placeholder"; // TODO: Need to assign the original image here
    fruit -> x = (rand() % (HEIGHT - 100 + 1)) + 100; // create a random x value between 100 and HEIGHT
    fruit -> y = WIDTH; // uses the display width of y
    fruit -> x_speed = (rand() % (10 - (-10) + 1)) + (-10); // create a random speed here between -10 and 10
    fruit -> y_speed = (rand() % (-60 - (-80) + 1)) + (-80); // create a random speed here -80 and -60
    fruit -> throw = false;
    fruit -> t = 0;
    fruit -> hit = false;
    fruit -> next = NULL;

    // Return the next random floating point number in the range [0.0, 1.0) to keep the fruits inside the gameDisplay
    // TODO: Check what this means
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

    // continous loop for the game
    while (GameRunning){

        // initialization of timer 3
        // TODO: Create a header file that will use the timer 3 function here
        // init_tim3();

        // checks if the game is over
        if (gameOver){
            // if the game is stil in the first round, display the game over screen and update the round
            if (firstRound){
                show_gameover_screen();
                firstRound = false;
            }

            gameOver = false;
            playerLives = 3;

            // TODO: Need to change this display to draw the lives for us
            // Use what Bailey said and change it to draw a black box over the image lives
            // draw_lives(gameDisplay, 690, 5, player_lives, 'images/red_lives.png');

            score = 0; // updates the score back to 0
        }

        // TODO: Need some way to check whenever the game is over
        // for event in pygame.event.get():
        //     # checking for closing window
        //     if event.type == pygame.QUIT:
        //         game_running = False

        // TODO: Need to change this to incorporate functions from main
        // gameDisplay.blit(background, (0, 0))
        // gameDisplay.blit(score_text, (0, 0))
        // draw_lives(gameDisplay, 690, 5, player_lives, 'images/red_lives.png')

        // make sure that this is the first node in the linked list
        Fruit *current_fruit = fruits;
        char *new_image; // the new image that will be displayed if either a bomb or fruit is swiped

        // loops through every fruit in the fruits linked list
        while (current_fruit != NULL){

            // checks if the fruit should be thrown
            if ((current_fruit -> throw) == true){
                current_fruit -> x += current_fruit -> x_speed;       // increases the fruits x coordinate by x_speed
                current_fruit -> y += current_fruit -> y_speed;       // increases the fruits y coorindate by y_speed
                current_fruit -> y_speed += (1 * current_fruit->t); // changes the y-trajectory of the fruit
                current_fruit -> t += 1;                            // changes the trajectory speed for the next iteration

                if (current_fruit -> y <= WIDTH){
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
                    if (current_fruit -> name == 'bomb'){

                        // user loses a life if a bomb is swiped
                        playerLives = playerLives - 1;

                        // checks if the player has no more lives
                        if (!playerLives){
                            // convers up all remaining lives
                            hide_cross_lives(690, 15);
                        }

                        // checks if the player only has one life
                        else if (playerLives == 1){
                            // covers up the other two lives
                            hide_cross_lives(725, 15);
                        }

                        else if (playerLives == 2){
                            // overs up one life
                            hide_cross_lives(760, 15);
                        }

                        // if the user clicks bombs for three time, GAME OVER message should be displayed and the window should be reset
                        if (playerLives < 0){
                            show_gameover_screen();
                            gameOver = true;
                        }

                        // updates the new image for the bomb after explosion
                        // TODO: Change this location
                        new_image = "images/explosion.png";
                    }

                    // indication that a fruit was swiped
                    else{
                        // TODO Need to change this to open whichever image it was when the fruit gets cut
                        // updates a new imgage for the fruit after explosion
                        new_image = "images/" + "half_" + current_fruit->name + ".png"; //TODO: Need to fix this
                    }

                    // updates the fruit's new image after being swiped along with the speed in the x direction
                    current_fruit -> image = new_image;
                    current_fruit -> x_speed += 10;

                    if (current_fruit -> name == 'bomb'){
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
