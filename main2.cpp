#include <raylib.h>
#include <math.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>

const int screenWidth = 800;
const int screenHeight = 600;

// Game states
typedef enum GameScreen { MENU, GAMEPLAY, GAME_OVER } GameScreen;

GameScreen currentScreen = MENU;

bool gameRunning = false;
bool startAnimation = false;
bool endAnimation = false;
bool firstSpin = true; // Flag to check if it's the first spin
float wheelRotation = 0.0f;
float rotationSpeed = 0.0f;
int selectedSection = -1;
int numSections = 8; // Default for medium mode
int winCount = 0;
int lossCount = 0;
int remainingChances = 4; // Default for medium mode

bool randomWin = false; // Flag to track random win/loss
int secretNumber = -1; // Secret number to decide win condition
int userGuess = -1; // User's guess
std::vector<int> wheelNumbers; // Vector to store unique random numbers for the wheel

void SetDifficulty(const char *difficulty)
{
    if (strcmp(difficulty, "easy") == 0) {
        numSections = 6;
        rotationSpeed = 3.0f; // Slower spin
        remainingChances = 5; // 5 attempts in easy mode
    } else if (strcmp(difficulty, "medium") == 0) {
        numSections = 8;
        rotationSpeed = 5.0f; // Moderate spin
        remainingChances = 4; // 4 attempts in medium mode
    } else if (strcmp(difficulty, "hard") == 0) {
        numSections = 12;
        rotationSpeed = 7.0f; // Faster spin
        remainingChances = 3; // 3 attempts in hard mode
    }
}

void GenerateWheelNumbers()
{
    wheelNumbers.clear();
    for (int i = 0; i < numSections; i++) {
        wheelNumbers.push_back(i);  // Create a sequence of numbers for the wheel sections
    }
    std::random_shuffle(wheelNumbers.begin(), wheelNumbers.end());  // Shuffle the numbers to ensure randomness
}

void UpdateWheel()
{
    if (gameRunning) {
        wheelRotation += rotationSpeed;

        // Slow down the wheel over time
        rotationSpeed -= 0.005f;

        if (rotationSpeed <= 0.0f) {
            rotationSpeed = 0.0f;
            gameRunning = false;
            endAnimation = true;

            // Randomly select a section based on the final wheelRotation value
            selectedSection = (int)((fmod(wheelRotation, 360.0f) / 360.0f) * numSections);

            // Determine win or loss based on the user's guess
            if (wheelNumbers[selectedSection] == userGuess) {
                winCount++;
            } else {
                lossCount++;
            }

            // Decrease remaining chances
            remainingChances--;

            if (remainingChances <= 0) {
                currentScreen = GAME_OVER; // Transition to Game Over screen
            }
        }
    }
}

void StartGame()
{
    if (!gameRunning && !startAnimation && remainingChances > 0) {
        srand(time(0)); // Seed random number generator
        wheelRotation = 0.0f;
        rotationSpeed = 5.0f; // Initial spin speed
        gameRunning = true;
        selectedSection = -1;
        startAnimation = true;
        firstSpin = false; // Reset first spin flag after the first spin

        GenerateWheelNumbers(); // Generate unique random numbers for the wheel
        secretNumber = rand() % numSections; // Randomly select the secret number from the available sections

        // Prompt user for their guess
        userGuess = GetRandomValue(0, numSections - 1); // Replace this with actual user input logic
    }
}

void DrawWheel()
{
    Vector2 center = { screenWidth / 2, screenHeight / 2 };
    float radius = 200;

    // Draw sections of the wheel with digits
    for (int i = 0; i < numSections; i++) {
        float startAngle = 360.0f / numSections * i + wheelRotation;
        float endAngle = 360.0f / numSections * (i + 1) + wheelRotation;
        float midAngle = (startAngle + endAngle) / 2;

        DrawCircleSector(center, radius, startAngle, endAngle, 10, (i % 2 == 0) ? RED : BLUE);

        // Draw digits in the center of each section
        float textX = center.x + (radius - 40) * cos(DEG2RAD * midAngle);
        float textY = center.y + (radius - 40) * sin(DEG2RAD * midAngle);
        DrawText(TextFormat("%d", wheelNumbers[i]), textX - 10, textY - 10, 20, WHITE);

        // Draw arrow inside the digit number
        DrawTriangle((Vector2){textX - 5, textY - 5}, (Vector2){textX + 5, textY - 5}, (Vector2){textX, textY + 5}, YELLOW);
    }

    // Draw the pointer at the top center
    Vector2 pointer = { screenWidth / 2, screenHeight / 2 - radius - 30 };
    DrawTriangle(pointer, (Vector2){ pointer.x - 10, pointer.y + 20 }, (Vector2){ pointer.x + 10, pointer.y + 20 }, WHITE);

    // Display selected section
    if (!gameRunning && selectedSection >= 0) {
        DrawText(TextFormat("Selected section: %d", wheelNumbers[selectedSection]), screenWidth / 2 - 100, screenHeight - 50, 20, GREEN);
    }

    // Display number of wins and losses
    DrawText(TextFormat("Wins: %d", winCount), screenWidth / 2 - 100, screenHeight - 100, 20, YELLOW);
    DrawText(TextFormat("Losses: %d", lossCount), screenWidth / 2 + 20, screenHeight - 100, 20, RED);

    // Display remaining chances
    DrawText(TextFormat("Remaining chances: %d", remainingChances), screenWidth / 2 - 80, screenHeight - 70, 20, WHITE);
}

void DrawStartAnimation()
{
    static int frameCount = 0;
    frameCount++;
    if (frameCount < 60) {
        DrawText("Starting...", screenWidth / 2 - 50, screenHeight / 2, 20, WHITE);
    } else {
        startAnimation = false;
        frameCount = 0;
    }
}

void DrawMenuScreen()
{
    DrawText("Casino Wheel Game", screenWidth / 2 - 130, screenHeight / 2 - 80, 40, YELLOW);
    DrawText("Press SPACE to Start", screenWidth / 2 - 150, screenHeight / 2, 30, WHITE);
}

void DrawGameOverScreen()
{
    DrawText("Game Over", screenWidth / 2 - 100, screenHeight / 2 - 50, 40, RED);
    DrawText(TextFormat("Wins: %d", winCount), screenWidth / 2 - 100, screenHeight / 2, 30, YELLOW);
    DrawText(TextFormat("Losses: %d", lossCount), screenWidth / 2 - 100, screenHeight / 2 + 50, 30, RED);
    DrawText("Press ENTER to Play Again", screenWidth / 2 - 150, screenHeight / 2 + 120, 20, WHITE);
}

int main()
{
    // Initialize the window and game
    InitWindow(screenWidth, screenHeight, "Casino Wheel Game");
    SetTargetFPS(60); // Set the frame rate

    // Main game loop
    while (!WindowShouldClose())
    {
        // Menu screen logic
        if (currentScreen == MENU)
        {
            if (IsKeyPressed(KEY_SPACE)) {
                currentScreen = GAMEPLAY;
            }
        }

        // Game Over screen logic
        if (currentScreen == GAME_OVER)
        {
            if (IsKeyPressed(KEY_ENTER)) {
                // Reset game state and go back to the main menu
                currentScreen = MENU;
                winCount = 0;
                lossCount = 0;
                remainingChances = 4; // Reset chances
                firstSpin = true; // Reset first spin flag
            }
        }

        // Gameplay screen logic
        if (currentScreen == GAMEPLAY)
        {
            // Start game on space press
            if (IsKeyPressed(KEY_SPACE)) {
                StartGame();
            }

            // Change difficulty based on keys
            if (IsKeyPressed(KEY_ONE)) SetDifficulty("easy");
            if (IsKeyPressed(KEY_TWO)) SetDifficulty("medium");
            if (IsKeyPressed(KEY_THREE)) SetDifficulty("hard");

            UpdateWheel();
        }

        // Drawing logic
        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == MENU) {
            DrawMenuScreen();
        } else if (currentScreen == GAMEPLAY) {
            DrawText("Press SPACE to spin the wheel", 20, 20, 20, WHITE);
            DrawText("Press 1 for Easy, 2 for Medium, 3 for Hard", 20, 50, 20, WHITE);

            DrawWheel();

            if (startAnimation) {
                DrawStartAnimation();
            }
        } else if (currentScreen == GAME_OVER) {
            DrawGameOverScreen();
        }

        EndDrawing();
    }

    CloseWindow(); // Close the window after exiting the loop
    return 0;
}
