#include <SDL2/SDL.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GRID_WIDTH 40
#define GRID_HEIGHT 30
#define PIXEL_SCALE 20
#define SCREEN_WIDTH (GRID_WIDTH * PIXEL_SCALE)
#define SCREEN_HEIGHT (GRID_HEIGHT * PIXEL_SCALE)

int grid[GRID_HEIGHT][GRID_WIDTH];

enum Directions {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

enum Tiles {
    WALL,
    GOAL,
    EMPTY,
};

#define INPUT_BUFFER_SIZE 5
typedef struct InputBuffer
{
    int oldest;
    int len;
    SDL_Keysym inputs[INPUT_BUFFER_SIZE];
} InputBuffer;
InputBuffer input_buf = {0};

void input_buf_queue(SDL_Keysym key)
{
    assert(input_buf.len < INPUT_BUFFER_SIZE);

    int index = (input_buf.oldest + input_buf.len) % INPUT_BUFFER_SIZE;
    input_buf.inputs[index] = key;
    input_buf.len++;
}

SDL_Keysym input_buf_dequeue()
{
    assert(input_buf.len);

    SDL_Keysym key = input_buf.inputs[input_buf.oldest];
    input_buf.oldest = (input_buf.oldest + 1) % INPUT_BUFFER_SIZE;
    input_buf.len--;
    return key;
}

typedef struct Snake
{
    int y;
    int x;
    int length;
    enum Directions direction;
} Snake;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
Snake snake;


void add_goal() {
    while (1) {
        int y = rand() % GRID_HEIGHT;
        int x = rand() % GRID_WIDTH;

        if (grid[y][x] == EMPTY) {
            grid[y][x] = GOAL;
            return;
        }
    }
}

void setup_grid() {
    // Sets walls and empty tiles
    for (size_t y = 0; y < GRID_HEIGHT; ++y) {
        for (size_t x = 0; x < GRID_WIDTH; ++x) {
            switch (y) {
            case 0:
            case (GRID_HEIGHT - 1):
                grid[y][x] = WALL;
                continue;
            }
            switch (x) {
            case 0:
            case (GRID_WIDTH - 1):
                grid[y][x] = WALL;
                continue;
            }

            grid[y][x] = EMPTY;
        }
    }

    // Set snake head and body tiles
    grid[snake.y][snake.x] = EMPTY + snake.length;
    switch (snake.direction) {
    case UP:
        grid[snake.y + 1][snake.x] = EMPTY + snake.length - 1;
        break;
    case DOWN:
        grid[snake.y - 1][snake.x] = EMPTY + snake.length - 1;
        break;
    case LEFT:
        grid[snake.y][snake.x + 1] = EMPTY + snake.length - 1;
        break;
    case RIGHT:
        grid[snake.y][snake.x - 1] = EMPTY + snake.length - 1;
        break;
    }

    add_goal();
}

void setup_window() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        printf("SDL Could not initialize. SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("Snake",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              0);

    if (window == NULL) {
        printf("SDL failed to create window. SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        printf("SDL failed to create renderer. SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void render_grid() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_Rect tile;
    for (size_t y = 0; y < GRID_HEIGHT; ++y) {
        for (size_t x = 0; x < GRID_WIDTH; ++x) {
            switch (grid[y][x]) {
            case WALL: {
                tile.y = y * PIXEL_SCALE;
                tile.x = x * PIXEL_SCALE;
                tile.w = PIXEL_SCALE;
                tile.h = PIXEL_SCALE;

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &tile);
                break;
            }
            case GOAL: {
                tile.y = y * PIXEL_SCALE;
                tile.x = x * PIXEL_SCALE;
                tile.w = PIXEL_SCALE;
                tile.h = PIXEL_SCALE;

                SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &tile);
                break;
            }
            case EMPTY: {
                break;
            }
            default: {
                // snake parts
                // offset is used to change the size of the snake body.
                // gives the snake a slightly separated look
                int offset = PIXEL_SCALE - (PIXEL_SCALE * .9);
                tile.y = (y * PIXEL_SCALE) + offset;
                tile.x = (x * PIXEL_SCALE) + offset;
                tile.w = PIXEL_SCALE - (offset * 2);
                tile.h = PIXEL_SCALE - (offset * 2);

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &tile);
                break;
                break;
            }
            }
        }
    }


    SDL_RenderPresent(renderer);
}

void handle_keypress(SDL_Keysym key)
{
    switch (key.scancode) {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_K:
    case SDL_SCANCODE_UP:
        if (snake.direction != DOWN) {
            snake.direction = UP;
        }
        break;
    case SDL_SCANCODE_S:
    case SDL_SCANCODE_J:
    case SDL_SCANCODE_DOWN:
        if (snake.direction != UP) {
            snake.direction = DOWN;
        }
        break;
    case SDL_SCANCODE_A:
    case SDL_SCANCODE_H:
    case SDL_SCANCODE_LEFT:
        if (snake.direction != RIGHT) {
            snake.direction = LEFT;
        }
        break;
    case SDL_SCANCODE_D:
    case SDL_SCANCODE_L:
    case SDL_SCANCODE_RIGHT:
        if (snake.direction != LEFT) {
            snake.direction = RIGHT;
        }
        break;
    default:
        break;
    }
}

void tick()
{
    if (input_buf.len) {
        handle_keypress(input_buf_dequeue());
    }

    for (size_t y = 0; y < GRID_HEIGHT; ++y) {
        for (size_t x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x] > EMPTY) {
                --grid[y][x];
            }
        }
    }

    switch (snake.direction) {
    case UP:
        snake.y--;
        break;
    case DOWN:
        snake.y++;
        break;
    case LEFT:
        snake.x--;
        break;
    case RIGHT:
        snake.x++;
        break;
    }

    switch (grid[snake.y][snake.x]) {
    case EMPTY:
        grid[snake.y][snake.x] = EMPTY + snake.length;
        break;
    case GOAL:
        ++snake.length;
        grid[snake.y][snake.x] = EMPTY + snake.length;
        add_goal();
        break;
    default:
        printf("Game over. Score: %d\n", snake.length - 2);
        SDL_DestroyWindow(window);
        exit(0);
    }
}

int main()
{
    atexit(SDL_Quit);

    srand(time(0));

    snake = (Snake){
        .y = (GRID_HEIGHT / 2),
        .x = (GRID_WIDTH / 2),
        .length = 2,
        .direction = rand() % 4,
    };


    setup_grid();
    setup_window();

    render_grid();

    SDL_Event event;
    int has_quit = 0;
    while (1) {
        while (!has_quit && SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                has_quit = 1;
                break;
            case SDL_KEYDOWN:
                input_buf_queue(event.key.keysym);
                break;
            }
        }

        if (has_quit) {
            break;
        }

        tick();
        render_grid();
        SDL_Delay(80);
    }

    SDL_DestroyWindow(window);
    return 0;
}
