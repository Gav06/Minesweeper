#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define DISPLAY_WIDTH_PX 480
#define DISPLAY_HEIGHT_PX 640

// Board size in pixels
#define BOARD_LENGTH_PX DISPLAY_WIDTH_PX
// Length of one axis of the board
#define BOARD_SIZE_1D 16
// Total number of cells
#define BOARD_SIZE_2D (BOARD_SIZE_1D * BOARD_SIZE_1D)
// Total number of mines
#define MINE_COUNT 40
// Size of Cell in pixels
#define CELL_SIZE_PX (BOARD_LENGTH_PX / BOARD_SIZE_1D)
// The y value of where the board starts
#define BOARD_BOUNDARY (DISPLAY_HEIGHT_PX - BOARD_LENGTH_PX)

#define SPRITE_BLANK_TILE ((Rectangle) { 0, 0, 16, 16 })
#define SPRITE_CLEAR_TILE ((Rectangle) { 16, 0, 16, 16 })
#define SPRITE_FLAG_TILE ((Rectangle) { 32, 0, 16, 16 })
#define SPRITE_MINE_TILE ((Rectangle) { 48, 0, 16, 16 })

typedef struct {
    uint8_t dangerLevel;
    uint8_t col;
    uint8_t row;
    bool isMine;
    bool isRevealed;
    bool isFlagged;
} Cell_t;

typedef struct {
    bool gameOver;
    bool isBoardGenerated;
    // number of flags remaining; available to place
    uint8_t flagCount;
    Texture2D texAtlas;
    Cell_t **board;
} GameState_t;

// Generates the default, or clear board. Basically copying every array position with a default struct
void clear_board(Cell_t **board) {
        // Set every cell to a "default" cell
    for (int row = 0; row < BOARD_SIZE_1D; row++) {
        for (int col = 0; col < BOARD_SIZE_1D; col++) {
            board[row][col] = (Cell_t) {
                .dangerLevel = 0,
                .col = col,
                .row = row,
                .isMine = false,
                .isRevealed = false,
                .isFlagged = false
            };
        }
    }
}

void recursive_flood_clear(GameState_t *gameState, int row, int col);

void recursive_flood_clear(GameState_t *gameState, int row, int col) {
    Cell_t *currCell = &gameState->board[row][col];
    if (currCell->isRevealed 
        || currCell->isFlagged
        || row < 0 
        || row > 15 
        || col < 0 
        || col > 15) {
        return;
    }

    currCell->isRevealed = true;
    if (currCell->isMine) {
        gameState->gameOver = true;
        return;
    }

    if (currCell->dangerLevel > 0) {
        return;
    }
    
    // check neighbors and repeat

    // up
    if (col - 1 >= 0) {
        recursive_flood_clear(gameState, row, col - 1);
    }
    // down
    if (col + 1 <= 15) {
        recursive_flood_clear(gameState, row, col + 1);
    }
    // left
    if (row - 1 >= 0) {
        recursive_flood_clear(gameState, row - 1, col);
    }
    // right
    if (row + 1 <= 15) {
        recursive_flood_clear(gameState, row + 1, col);
    }
}
// This function assumes the board has already been allocated in memory.
// Generates the mines with a buffer zone around the given position.
void gen_minefield(Cell_t **board, int row, int col) {
    // Place the mines
    int placed = 0;
    while (placed < MINE_COUNT) {
        // Silly edge condition
        if (placed > BOARD_SIZE_2D) {
            break;
        }

        int randRow = rand() % BOARD_SIZE_1D;
        int randCol = rand() % BOARD_SIZE_1D;

        // Area of "safety" where no mines generate around initial mouse click
        // Or whatever position the (row, col) params are for

        if (row != -1 && col != -1) {
            const int safeDist = 3;

            if (abs(randRow - row) < safeDist && abs(randCol - col) < safeDist) {
                continue;
            }
        }

        if (!board[randRow][randCol].isMine) {
            board[randRow][randCol].isMine = true;
            placed++;
        }
    }

    // Loop through each cell and set a danger level (the number)
    for (int row = 0; row < BOARD_SIZE_1D; row++) {
        for (int col = 0; col < BOARD_SIZE_1D; col++) {
            Cell_t *cell = &board[row][col];

            if (cell == NULL) {
                continue;
            }

            if (cell->isMine) {
                continue;
            }

            // check every adjacent cell
            int dangerLevel = 0;

            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    int checkRow = cell->row - 1 + r;
                    int checkCol = cell->col - 1 + c;

                    if (checkRow < 0 
                        || checkRow > 15 
                        || checkCol < 0 
                        || checkCol > 15) {
                        continue;
                    }

                    Cell_t *neighbor = &board[checkRow][checkCol];
                    
                    if (neighbor != NULL && neighbor->isMine) {
                        dangerLevel++;
                    }
                }
            }

            cell->dangerLevel = dangerLevel;
        }
    }
}

Rectangle find_cell_sprite(Cell_t *cell) {
    if (cell->isRevealed) {
        if (cell->isMine) {
            return SPRITE_MINE_TILE;
        } else {
            return SPRITE_CLEAR_TILE;
        }
    } else {
        if (cell->isFlagged) {
            return SPRITE_FLAG_TILE;
        } else {
            return SPRITE_BLANK_TILE;
        }
    }
}

Color get_danger_color(int dangerLevel) {
    switch (dangerLevel) {
        case 1:
            return BLUE;
        case 2:
            return GREEN;
        case 3:
            return RED;
        case 4:
            return DARKBLUE;
        case 5:
            return MAROON;
        case 6:
            return (Color) { 0, 255, 255, 255 }; // CYAN
        case 7:
            return BLACK;
        case 8:
            return DARKGRAY;
        default:
            return PINK;
    }
}

void draw_minefield(GameState_t *gameState) {
    for (int row = 0; row < BOARD_SIZE_1D; row++) {
        for (int col = 0; col < BOARD_SIZE_1D; col++) {
            Cell_t *currCell = &gameState->board[row][col];
            
            // find which texture to use
            Rectangle sprite = find_cell_sprite(currCell);
            
            const int cellX = currCell->col * CELL_SIZE_PX;
            const int cellY = (currCell->row * CELL_SIZE_PX) + (BOARD_BOUNDARY);
            const Rectangle cellPos = {
                .x = cellX,
                .y = cellY,
                .height = CELL_SIZE_PX,
                .width = CELL_SIZE_PX
            };

            DrawTexturePro(gameState->texAtlas, sprite, cellPos, (Vector2) { 0, 0 }, 0.0f, RAYWHITE);

            if (!currCell->isRevealed || currCell->dangerLevel <= 0) {
                continue;
            }
            
            // convert the int to ascii (at least for only the first digit)
            char digit = currCell->dangerLevel + '0';
            char line[] = { digit, '\0' };
            DrawText(line, cellX + 10, cellY + 6, 24, get_danger_color(currCell->dangerLevel));
        }
    }
}

// load textures & generate minefield pattern (temporarily)
void init(GameState_t *gameState) {
    if (!gameState) {
        fprintf(stderr, "Failed to malloc game state.\n");
        exit(1);
    }

    gameState->isBoardGenerated = false;
    gameState->flagCount = MINE_COUNT;
    gameState->texAtlas = LoadTexture("atlas.png");

    // Allocate rows
    gameState->board = malloc(sizeof(Cell_t*) * BOARD_SIZE_1D);
    if (!gameState->board) {
        fprintf(stderr, "Failed to malloc rows of board\n");
    }

    // Allocate columns
    for (int i = 0; i < BOARD_SIZE_1D; i++) {
        gameState->board[i] = malloc(sizeof(Cell_t) * BOARD_SIZE_1D);

        if (!gameState->board[i]) {
            fprintf(stderr, "Failed to malloc column of board\n");
        }
    }

    // Seed our RNG for all board generations
    srand((unsigned int) time(NULL));
    clear_board(gameState->board);
}

void update(GameState_t *gameState) {
    Vector2 mousePos = GetMousePosition();

    // find current cell that mouse is over
    if (mousePos.y < BOARD_BOUNDARY) {
        return;
    }

    const int row = (mousePos.y - BOARD_BOUNDARY) / CELL_SIZE_PX;
    const int col = mousePos.x / CELL_SIZE_PX;

    // make sure we arent out of bounds
    if (row < 0 
        || row > 15 
        || col < 0 
        || col > 15) {
        return;
    }

    Cell_t *targetCell = &gameState->board[row][col];

    // check mouse clicks, left click to clear cells and right click to flag cells
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        // Initial mouse click
        if (!gameState->isBoardGenerated) {
            gen_minefield(gameState->board, row, col);
            gameState->isBoardGenerated = true;
        }

        if (!targetCell->isFlagged) {
            recursive_flood_clear(gameState, row, col);
        }

    } else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        if (targetCell->isRevealed || gameState->flagCount <= 0) {
            return;
        }

        if (targetCell->isFlagged) {
            gameState->flagCount++;
            targetCell->isFlagged = false;
        } else {
            gameState->flagCount--;
            targetCell->isFlagged = true;
        }
    }

    /**
     * Debug features:
     * 
     * Pressing 'R' will regenerate the board with new pattern
     * 
     * Pressing 'T' will reveal all squares, without ending the game
     * 
     */

    if (IsKeyReleased(KEY_R)) {
        if (!gameState->isBoardGenerated) {
            return;
        }

        // clears previous board
        clear_board(gameState->board);
        // places mines
        gen_minefield(gameState->board, -1, -1);
    }

    if (IsKeyReleased(KEY_T)) {
        if (!gameState->isBoardGenerated) {
            return;
        }

        // reveal every cell
        for (int row = 0; row < BOARD_SIZE_1D; row++) {
            for (int col = 0; col < BOARD_SIZE_1D; col++) {
                gameState->board[row][col].isRevealed = true;
            }
        }
    }
}

void render(GameState_t *gameState) {
    ClearBackground(RAYWHITE);
    
    char buf[24];
    snprintf(buf, sizeof(buf), "Flags left: %d", gameState->flagCount);
    DrawText(buf, 2, 2, 24, BLACK);

    // mine
    draw_minefield(gameState);
}

void cleanup(GameState_t *gameState) {
    UnloadTexture(gameState->texAtlas);

    // Free cells
    for (int i = 0; i < BOARD_SIZE_1D; i++) {
        free(gameState->board[i]);
    }
    free(gameState->board);
    free(gameState);
}

int main() {
    InitWindow(DISPLAY_WIDTH_PX, DISPLAY_HEIGHT_PX, "Minesweeper");

    // Allocate and initialize game
    GameState_t *gameState = malloc(sizeof(GameState_t));
    init(gameState);

    while (!WindowShouldClose()) {
        // tick logic
        update(gameState);
        // draw logic
        BeginDrawing();
        render(gameState);
        EndDrawing();
    }

    cleanup(gameState);
    return 0;
}