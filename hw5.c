#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
struct gameState {
    FILE * map;
    int * position;
    char * hasSeen;
};

struct gameMap {
    int cols;
    int rows;
    char * entries;
};

void printMap(struct gameMap map, struct gameState state);
struct gameMap generateMap(char * filename);
int generateStartPosition(struct gameMap map);
struct gameState initializeGameState(struct gameMap map);
void updateHasSeen(struct gameState *state, struct gameMap map);
int movePlayer(struct gameState *state, struct gameMap map, const char *direction);

int main(void) {
    srand(time(NULL));

    struct gameMap map1 = generateMap("map1.txt");
    printf("Rows: %d\nCols: %d\n", map1.rows, map1.cols);

    struct gameState gameState = initializeGameState(map1);

    char input[20];
    while (1) {
        // Display the map
        printMap(map1, gameState);

        // Prompt for input
        printf("current position: %d,%d\n", gameState.position[0]+1, gameState.position[1]+1);
        printf("Enter direction ('north', 'south', 'east', 'west') or 'quit' to exit: ");
        scanf("%s", input);

        // Handle user input
        if (strcmp(input, "quit") == 0) {
            break;
        }

        // Attempt to move the player
        if (movePlayer(&gameState, map1, input)) {
            printf("Moved %s.\n", input);
        }
    }

    // Clean up
    free(map1.entries);
    free(gameState.position);
    free(gameState.hasSeen);

    return 0;
}




struct gameMap generateMap(char *filename) {
    struct gameMap map = {0, 0, NULL}; // Initialize map fields to default values

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening the file");
        return map;
    }

    char line[1024];

    // First pass: Calculate rows and columns
    while (fgets(line, sizeof(line), file)) {
        map.rows++;
        if (map.rows == 1) {
            // Handle newline and carriage return
            if (line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = '\0'; // Strip newline
            }
            if (line[strlen(line) - 1] == '\r') {
                line[strlen(line) - 1] = '\0'; // Strip carriage return
            }
            map.cols = strlen(line); // Set column count
        }
    }

    if (map.rows == 0 || map.cols == 0) {
        printf("Error: Map file is empty or invalid.\n");
        fclose(file);
        return map;
    }

    // allocate memory for map.entries
    map.entries = malloc(map.rows * map.cols * sizeof(char));
    if (map.entries == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return map;
    }

    // populate map.entries
    rewind(file);
    int row = 0;
    while (fgets(line, sizeof(line), file)) {
        // strip newline and carriage return
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        if (line[strlen(line) - 1] == '\r') {
            line[strlen(line) - 1] = '\0';
        }
        for (int col = 0; col < map.cols; col++) {
            map.entries[row * map.cols + col] = line[col];
        }
        row++;
    }

    fclose(file);
    return map;
}


void printMap(struct gameMap map, struct gameState state) {
    for (int i = 0; i < map.rows; i++) {
        for (int j = 0; j < map.cols; j++) {
            int index = i * map.cols + j;

            if (i == state.position[0] && j == state.position[1]) {
                // Print 'x' at the player's current position
                printf("x");
            } else if (state.hasSeen[index]) {
                // Print actual map character for seen tiles
                printf("%c", map.entries[index]);
            } else {
                // Print placeholder for unseen tiles
                printf(" ");
            }
        }
        printf("\n");
    }
}



int generateStartPosition(struct gameMap map) {
    int index;
    int valid;

    do {
        index = rand() % (map.rows * map.cols); // Random index
        valid = 0;

        // Check if the starting position is '.'
        if (map.entries[index] == '.') {
            int row = index / map.cols;
            int col = index % map.cols;

            // Check if at least one adjacent tile is '.'
            for (int dr = -1; dr <= 1 && !valid; dr++) {
                for (int dc = -1; dc <= 1 && !valid; dc++) {
                    if (dr == 0 && dc == 0) continue; // Skip the starting tile itself
                    int newRow = row + dr;
                    int newCol = col + dc;

                    // Check bounds
                    if (newRow >= 0 && newRow < map.rows &&
                        newCol >= 0 && newCol < map.cols) {
                        int adjacentIndex = newRow * map.cols + newCol;
                        if (map.entries[adjacentIndex] == '.') {
                            valid = 1; // Found a valid adjacent tile
                        }
                    }
                }
            }
        }
    } while (!valid); // Repeat until a valid starting position is found

    return index;
}


void updateHasSeen(struct gameState *state, struct gameMap map) {
    int row = state->position[0];
    int col = state->position[1];

    // Mark the 3x3 square around the current position
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            int newRow = row + dr;
            int newCol = col + dc;

            // Check bounds to avoid accessing out-of-range indices
            if (newRow >= 0 && newRow < map.rows && newCol >= 0 && newCol < map.cols) {
                int index = newRow * map.cols + newCol;
                state->hasSeen[index] = 1; // Mark tile as seen
            }
        }
    }
}



struct gameState initializeGameState(struct gameMap map) {
    struct gameState state;
    state.map = NULL;

    state.position = malloc(2 * sizeof(int));
    state.hasSeen = calloc(map.rows * map.cols, sizeof(char));

    if (state.position == NULL || state.hasSeen == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    int startIndex = generateStartPosition(map);
    state.position[0] = startIndex / map.cols;
    state.position[1] = startIndex % map.cols;

    updateHasSeen(&state, map);

    return state;
}

int movePlayer(struct gameState *state, struct gameMap map, const char *direction) {
    int row = state->position[0];
    int col = state->position[1];
    int newRow = row, newCol = col;

    // Determine new position based on direction
    if (strcmp(direction, "north") == 0) {
        newRow--;
    } else if (strcmp(direction, "south") == 0) {
        newRow++;
    } else if (strcmp(direction, "west") == 0) {
        newCol--;
    } else if (strcmp(direction, "east") == 0) {
        newCol++;
    } else {
        printf("Invalid direction. Please enter 'north', 'south', 'east', or 'west'.\n");
        return 0;
    }

    // Validate new position
    if (newRow >= 0 && newRow < map.rows &&
        newCol >= 0 && newCol < map.cols &&
        map.entries[newRow * map.cols + newCol] == '.') {
        // Update position
        state->position[0] = newRow;
        state->position[1] = newCol;

        // Update visibility
        updateHasSeen(state, map);
        return 1;
    } else {
        printf("Invalid move. You can only move to '.' tiles.\n");
        return 0;
    }
}





// struct gameState loadFile(char * filename, int isNew){
//     struct gameState state;
//     state.map = fopen(filename, "r");
//     if (state.map == NULL) {
//         perror("Failed to open file");
//         state.position = NULL;
//         return state;
//     }
//     state.position = ;  // Initialize other fields as needed
//     return state;
// }