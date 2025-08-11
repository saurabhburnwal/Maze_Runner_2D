#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h> // Required for bool type
#include <math.h>    // Required for abs()

// --- Game Constants and Global Variables ---
#define LEVELS 3
#define ROWS 15
#define COLS 20

// Maze data
char maze[LEVELS][ROWS][COLS];
char items[LEVELS][ROWS][COLS];
bool revealed[LEVELS][ROWS][COLS]; // NEW: Tracks revealed tiles

// Player state
int playerX, playerY, playerZ;
int hasKey;
int score; // Player score

// Level data
int level_start_x[LEVELS];
int level_start_y[LEVELS];

// Maze generation helpers
int dx[] = { -2, 2, 0, 0 };
int dy[] = { 0, 0, -2, 2 };

// --- Maze Generation and Logic ---

void shuffle(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        int j = rand() % n;
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void initializeMaze(int level) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            maze[level][i][j] = '#';
            items[level][i][j] = ' ';
            revealed[level][i][j] = false; // Initialize revealed map
        }
    }
}

void generateMazeDFS(int level, int x, int y) {
    maze[level][x][y] = '.';
    int dirs[4] = { 0, 1, 2, 3 };
    shuffle(dirs, 4);

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[dirs[i]];
        int ny = y + dy[dirs[i]];
        if (nx > 0 && nx < ROWS - 1 && ny > 0 && ny < COLS - 1 && maze[level][nx][ny] == '#') {
            maze[level][x + dx[dirs[i]] / 2][y + dy[dirs[i]] / 2] = '.';
            generateMazeDFS(level, nx, ny);
        }
    }
}

void placeFeatures(int level) {
    int placed = 0;
    while (!placed) {
        int tx = (rand() % (ROWS - 2)) + 1;
        int ty = (rand() % (COLS - 2)) + 1;
        if (maze[level][tx][ty] == '.' && items[level][tx][ty] == ' ') {
            items[level][tx][ty] = 'T';
            placed = 1;
        }
    }
    if (level == 1) {
        placed = 0;
        while (!placed) {
            int kx = (rand() % (ROWS - 2)) + 1;
            int ky = (rand() % (COLS - 2)) + 1;
            if (maze[level][kx][ky] == '.' && items[level][kx][ky] == ' ') {
                items[level][kx][ky] = 'K';
                placed = 1;
            }
        }
    }
    if (level == 2) {
        placed = 0;
        while (!placed) {
            int dx_feat = (rand() % (ROWS - 2)) + 1;
            int dy_feat = (rand() % (COLS - 4)) + 3;
            if (maze[level][dx_feat][dy_feat] == '.') {
                maze[level][dx_feat][dy_feat] = 'D';
                placed = 1;
            }
        }
    }
}

void generateMaze() {
    level_start_x[0] = 1;
    level_start_y[0] = 1;

    for (int z = 0; z < LEVELS; z++) {
        initializeMaze(z);
        generateMazeDFS(z, 1, 1);
        placeFeatures(z);
    }

    for (int z = 0; z < LEVELS - 1; z++) {
        int sx, sy;
        do {
            sx = (rand() % (ROWS - 2)) + 1;
            sy = (rand() % (COLS - 2)) + 1;
        } while (maze[z][sx][sy] != '.');

        maze[z][sx][sy] = '>';
        maze[z + 1][sx][sy] = '<';
        level_start_x[z + 1] = sx;
        level_start_y[z + 1] = sy;
    }

    maze[0][1][1] = 'S';
    maze[LEVELS - 1][ROWS - 2][COLS - 2] = 'E';

    playerX = level_start_x[0];
    playerY = level_start_y[0];
    playerZ = 0;
    hasKey = 0;
    score = 1000; // Initialize score to 100
}

// --- Raylib Implementation ---

void DrawMaze(int tileSize);
void DrawHUD(const char* status);
void UpdateRevealedMap(int visionRadius);

int main(void) {
    // 1. Initialization
    const int tileSize = 50;
    const int screenWidth = COLS * tileSize;
    const int screenHeight = ROWS * tileSize + 100; // Extra space for HUD
    const int visionRadius = 2; // How many tiles the player can see around them

    InitWindow(screenWidth, screenHeight, "Maze Runner - The Explorer's Update");
    SetTargetFPS(60);

    srand(time(NULL));
    generateMaze();
    UpdateRevealedMap(visionRadius); // Initial reveal around the start point

    bool gameWon = false;
    char statusMessage[100] = "Use arrow keys to move. Find the exit!";
    
    // Movement timing variables
    float moveTimer = 0.0f;
    const float moveDelay = 0.25f; // Delay between moves in seconds (slowed down)

    // 2. Main Game Loop
    while (!WindowShouldClose()) {

        // Update movement timer
        moveTimer += GetFrameTime();

        // 3. Update (Input and Game Logic)
        if (!gameWon) {
            int nextX = playerX;
            int nextY = playerY;
            bool moved = false;

            // Check for continuous movement when keys are held down, but only if enough time has passed
            if (moveTimer >= moveDelay) {
                if (IsKeyDown(KEY_UP)) { nextX--; moved = true; }
                else if (IsKeyDown(KEY_DOWN)) { nextX++; moved = true; }
                else if (IsKeyDown(KEY_LEFT)) { nextY--; moved = true; }
                else if (IsKeyDown(KEY_RIGHT)) { nextY++; moved = true; }

                if (moved) {
                    moveTimer = 0.0f; // Reset timer when movement occurs
                }
            }

            if (moved) {
                char destination_tile = maze[playerZ][nextX][nextY];
                if (destination_tile == '#') {
                    sprintf_s(statusMessage, sizeof(statusMessage), "You hit a wall!");
                }
                else if (destination_tile == 'D' && !hasKey) {
                    sprintf_s(statusMessage, sizeof(statusMessage), "The door is locked! You need a key.");
                }
                else {
                    score--; // Decrease score by 1 for each step taken
                    if (score < 0) score = 0; // Prevent negative scores
                    playerX = nextX;
                    playerY = nextY;
                    UpdateRevealedMap(visionRadius); // Reveal new area after moving
                    
                    sprintf_s(statusMessage, sizeof(statusMessage), "Level: %d/%d | Score: %d", playerZ + 1, LEVELS, score);

                    if (destination_tile == 'D' && hasKey) {
                        score += 10; // Bonus points for unlocking door
                        sprintf_s(statusMessage, sizeof(statusMessage), "You unlocked the door! +10 points");
                        maze[playerZ][playerX][playerY] = '.';
                        hasKey = 0;
                    }
                    else if (destination_tile == '>') {
                        playerZ++;
                        score += 5; // Bonus points for going down stairs
                        UpdateRevealedMap(visionRadius); // Reveal new area on new level
                        sprintf_s(statusMessage, sizeof(statusMessage), "You went down stairs to level %d... +5 points", playerZ + 1);
                    }
                    else if (destination_tile == '<') {
                        playerZ--;
                        UpdateRevealedMap(visionRadius); // Reveal new area on new level
                        sprintf_s(statusMessage, sizeof(statusMessage), "You went up stairs to level %d...", playerZ + 1);
                    }
                    else if (destination_tile == 'E') {
                        score += 50; // Big bonus for completing the game
                        gameWon = true;
                    }

                    char item_tile = items[playerZ][playerX][playerY];
                    if (item_tile == 'K') {
                        hasKey = 1;
                        score += 20; // Bonus points for finding key
                        items[playerZ][playerX][playerY] = ' ';
                        sprintf_s(statusMessage, sizeof(statusMessage), "You found a key! +20 points");
                    }
                    else if (item_tile == 'T') {
                        score -= 10; // Additional penalty for hitting trap
                        if (score < 0) score = 0; // Don't let score go negative
                        items[playerZ][playerX][playerY] = ' ';
                        playerX = level_start_x[playerZ];
                        playerY = level_start_y[playerZ];
                        UpdateRevealedMap(visionRadius); // Reveal start area after trap
                        sprintf_s(statusMessage, sizeof(statusMessage), "It's a trap! Back to the start. -10 points");
                    }
                }
            }
        }
        else {
            sprintf_s(statusMessage, sizeof(statusMessage), "You reached the exit! Final Score: %d | Press [ENTER] to play again.", score);
            if (IsKeyPressed(KEY_ENTER)) {
                generateMaze();
                UpdateRevealedMap(visionRadius);
                gameWon = false;
                moveTimer = 0.0f; // Reset movement timer when starting new game
            }
        }

        // 4. Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        if (!gameWon) {
            DrawMaze(tileSize);
        }
        else {
            DrawText("CONGRATULATIONS!", screenWidth / 2 - MeasureText("CONGRATULATIONS!", 40) / 2, screenHeight / 2 - 40, 40, GOLD);
        }

        DrawHUD(statusMessage);

        EndDrawing();
    }

    // 5. De-Initialization
    CloseWindow();
    return 0;
}

// Function to update the 'revealed' map based on player position
void UpdateRevealedMap(int visionRadius) {
    for (int i = playerX - visionRadius; i <= playerX + visionRadius; i++) {
        for (int j = playerY - visionRadius; j <= playerY + visionRadius; j++) {
            // Check bounds to avoid going outside the maze array
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                revealed[playerZ][i][j] = true;
            }
        }
    }
}

void DrawMaze(int tileSize) {
    int visionRadius = 2; // How many tiles player can see

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Only draw tiles that have been revealed
            if (revealed[playerZ][i][j]) {
                Vector2 pos = { (float)j * tileSize, (float)i * tileSize };
                Color tileColor = DARKGRAY;
                char tileChar = maze[playerZ][i][j];
                char itemChar = items[playerZ][i][j];

                // Determine base tile color
                switch (tileChar) {
                case '.': tileColor = LIGHTGRAY; break;
                case 'S': tileColor = GREEN; break;
                case 'E': tileColor = PURPLE; break;
                case '>': tileColor = BROWN; break;
                case '<': tileColor = BROWN; break;
                case 'D': tileColor = DARKBROWN; break;
                default: tileColor = (Color){ 50, 50, 50, 255 }; break; // Darker wall
                }

                // Check if the tile is within the player's current vision
                bool inCurrentVision = (abs(playerX - i) <= visionRadius && abs(playerY - j) <= visionRadius);

                // If not in current vision, dim the color (memory)
                if (!inCurrentVision) {
                    tileColor = Fade(tileColor, 0.3f);
                }

                DrawRectangleV(pos, (Vector2) { tileSize, tileSize }, tileColor);

                // Only draw details if they are in the current line of sight
                if (inCurrentVision) {
                    if (tileChar == '>') DrawText("▼", pos.x + 15, pos.y + 10, 30, RAYWHITE);
                    if (tileChar == '<') DrawText("▲", pos.x + 15, pos.y + 10, 30, RAYWHITE);
                    if (tileChar == 'D') DrawText("D", pos.x + 15, pos.y + 10, 30, (Color) { 100, 60, 40, 255 });

                    Vector2 itemCenter = { pos.x + tileSize / 2, pos.y + tileSize / 2 };
                    if (itemChar == 'K') DrawCircleV(itemCenter, tileSize / 4, GOLD);
                    if (itemChar == 'T') DrawCircleV(itemCenter, tileSize / 4, RED);
                }
            }
            // If a tile has not been revealed, it's not drawn, so it remains BLACK (fog of war)
        }
    }
    // Draw player last so it's always on top
    DrawRectangle(playerY * tileSize, playerX * tileSize, tileSize, tileSize, BLUE);
}

void DrawHUD(const char* status) {
    int hudY = ROWS * 50;
    DrawRectangle(0, hudY, COLS * 50, 100, (Color) { 20, 20, 20, 255 });

    // Draw status message on first line
    DrawText(status, 20, hudY + 20, 20, RAYWHITE);

    // Display score on second line
    char scoreText[50];
    sprintf_s(scoreText, sizeof(scoreText), "Score: %d", score);
    DrawText(scoreText, 20, hudY + 45, 20, YELLOW);

    // Display key status on second line, right side
    const char* keyText = hasKey ? "Key: YES" : "Key: NO";
    Color keyColor = hasKey ? GOLD : GRAY;
    DrawText(keyText, COLS * 50 - MeasureText(keyText, 20) - 20, hudY + 45, 20, keyColor);
}
