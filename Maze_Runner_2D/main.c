#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define LEVELS 3
#define ROWS 15
#define COLS 20

char maze[LEVELS][ROWS][COLS];
char items[LEVELS][ROWS][COLS];
bool revealed[LEVELS][ROWS][COLS];

int playerX, playerY, playerZ;
int hasKey;
int score;

int level_start_x[LEVELS];
int level_start_y[LEVELS];

int dx[] = { -2, 2, 0, 0 };
int dy[] = { 0, 0, -2, 2 };

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
            revealed[level][i][j] = false;
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
    score = 1000;
}

void DrawMaze(int tileSize);
void DrawHUD(const char* status);
void UpdateRevealedMap(int visionRadius);

int main(void) {
    const int tileSize = 50;
    const int screenWidth = COLS * tileSize;
    const int screenHeight = ROWS * tileSize + 100;
    const int visionRadius = 2;

    InitWindow(screenWidth, screenHeight, "Maze Runner - The Explorer's Update");
    SetTargetFPS(60);

    srand(time(NULL));
    generateMaze();
    UpdateRevealedMap(visionRadius);

    bool gameWon = false;
    char statusMessage[100] = "Use arrow keys to move. Find the exit!";
    
    float moveTimer = 0.0f;
    const float moveDelay = 0.15f;

    while (!WindowShouldClose()) {

        moveTimer += GetFrameTime();

        if (!gameWon) {
            int nextX = playerX;
            int nextY = playerY;
            bool moved = false;

            if (moveTimer >= moveDelay) {
                if (IsKeyDown(KEY_UP)) { nextX--; moved = true; }
                else if (IsKeyDown(KEY_DOWN)) { nextX++; moved = true; }
                else if (IsKeyDown(KEY_LEFT)) { nextY--; moved = true; }
                else if (IsKeyDown(KEY_RIGHT)) { nextY++; moved = true; }

                if (moved) {
                    moveTimer = 0.0f;
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
                    score--;
                    if (score < 0) score = 0;
                    playerX = nextX;
                    playerY = nextY;
                    UpdateRevealedMap(visionRadius);
                    
                    sprintf_s(statusMessage, sizeof(statusMessage), "Level: %d/%d | Score: %d", playerZ + 1, LEVELS, score);

                    if (destination_tile == 'D' && hasKey) {
                        score += 10;
                        sprintf_s(statusMessage, sizeof(statusMessage), "You unlocked the door! +10 points");
                        maze[playerZ][playerX][playerY] = '.';
                        hasKey = 0;
                    }
                    else if (destination_tile == '>') {
                        playerZ++;
                        score += 5;
                        UpdateRevealedMap(visionRadius);
                        sprintf_s(statusMessage, sizeof(statusMessage), "You went down stairs to level %d... +5 points", playerZ + 1);
                    }
                    else if (destination_tile == '<') {
                        playerZ--;
                        UpdateRevealedMap(visionRadius);
                        sprintf_s(statusMessage, sizeof(statusMessage), "You went up stairs to level %d...", playerZ + 1);
                    }
                    else if (destination_tile == 'E') {
                        score += 50;
                        gameWon = true;
                    }

                    char item_tile = items[playerZ][playerX][playerY];
                    if (item_tile == 'K') {
                        hasKey = 1;
                        score += 20;
                        items[playerZ][playerX][playerY] = ' ';
                        sprintf_s(statusMessage, sizeof(statusMessage), "You found a key! +20 points");
                    }
                    else if (item_tile == 'T') {
                        score -= 10;
                        if (score < 0) score = 0;
                        items[playerZ][playerX][playerY] = ' ';
                        playerX = level_start_x[playerZ];
                        playerY = level_start_y[playerZ];
                        UpdateRevealedMap(visionRadius);
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
                moveTimer = 0.0f;
            }
        }

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

    CloseWindow();
    return 0;
}

void UpdateRevealedMap(int visionRadius) {
    for (int i = playerX - visionRadius; i <= playerX + visionRadius; i++) {
        for (int j = playerY - visionRadius; j <= playerY + visionRadius; j++) {
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                revealed[playerZ][i][j] = true;
            }
        }
    }
}

void DrawMaze(int tileSize) {
    int visionRadius = 2;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (revealed[playerZ][i][j]) {
                Vector2 pos = { (float)j * tileSize, (float)i * tileSize };
                Color tileColor = DARKGRAY;
                char tileChar = maze[playerZ][i][j];
                char itemChar = items[playerZ][i][j];

                switch (tileChar) {
                case '.': tileColor = LIGHTGRAY; break;
                case 'S': tileColor = GREEN; break;
                case 'E': tileColor = PURPLE; break;
                case '>': tileColor = BROWN; break;
                case '<': tileColor = BROWN; break;
                case 'D': tileColor = DARKBROWN; break;
                default: tileColor = (Color){ 50, 50, 50, 255 }; break;
                }

                bool inCurrentVision = (abs(playerX - i) <= visionRadius && abs(playerY - j) <= visionRadius);

                if (!inCurrentVision) {
                    tileColor = Fade(tileColor, 0.3f);
                }

                DrawRectangleV(pos, (Vector2) { tileSize, tileSize }, tileColor);

                if (inCurrentVision) {
                    if (tileChar == '>') DrawText("▼", pos.x + 15, pos.y + 10, 30, RAYWHITE);
                    if (tileChar == '<') DrawText("▲", pos.x + 15, pos.y + 10, 30, RAYWHITE);
                    if (tileChar == 'D') DrawText("D", pos.x + 15, pos.y + 10, 30, (Color) { 100, 60, 40, 255 });

                    Vector2 itemCenter = { pos.x + tileSize / 2, pos.y + tileSize / 2 };
                    if (itemChar == 'K') DrawCircleV(itemCenter, tileSize / 4, GOLD);
                    if (itemChar == 'T') DrawCircleV(itemCenter, tileSize / 4, RED);
                }
            }
        }
    }
    DrawRectangle(playerY * tileSize, playerX * tileSize, tileSize, tileSize, BLUE);
}

void DrawHUD(const char* status) {
    int hudY = ROWS * 50;
    DrawRectangle(0, hudY, COLS * 50, 100, (Color) { 20, 20, 20, 255 });

    DrawText(status, 20, hudY + 20, 20, RAYWHITE);

    char scoreText[50];
    sprintf_s(scoreText, sizeof(scoreText), "Score: %d", score);
    DrawText(scoreText, 20, hudY + 45, 20, YELLOW);

    const char* keyText = hasKey ? "Key: YES" : "Key: NO";
    Color keyColor = hasKey ? GOLD : GRAY;
    DrawText(keyText, COLS * 50 - MeasureText(keyText, 20) - 20, hudY + 45, 20, keyColor);
}
