/************************************************************************************
Simple project by me (github user:prentizD) trying to visualize some path finding algorithms with the use of raylib as a gameengine
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define SCREENWIDTH 1100
#define SCREENHEIGHT 900
#define TARGET_FPS 60
#define WEIGHTS 1
#define WALLWEIGHT 1000000
#define FONTSIZE 18
#define BUTTONHEIGHT 40
#define BUTTONWIDTH 130
#define FONT_RESOURCE "./../resources/OpenSans-Italic.ttf"
#define DRAWSPEED 4

// Stuct used to navigate in a 2D-grid
typedef struct position_t
{
    int row;
    int column;
} Position;

// struct to define a single Tile in the 2D-grid
typedef struct tile_t
{
    Position tile_position;
    Rectangle rectangle; // contains x, y, width and height information
    Color color;
    int f; // used in AStar: cost from start to this note + heuristic cost to the goal
    int g; // used in AStar: incremental travel cost (weight) from start to this note
    bool inOpenList;
    int weight;           // used in Dijkstra/AStar: cost to ,,travel" through this Tile
    int reach_cost;       // used in Dijkstra: cost to reach this tile from the start point
    bool finalized;       // used in all: if tile is already visited
    Position *neighbours; // used in all: all neightbour locations of this tile
    int neighbour_count;  // keep track of neighbour count for iteration
    Position predesessor; // used in all
} Tile;

// Queue node to track a tile position
typedef struct queue_node_t
{
    Position tile_position;
    struct queue_node_t *next;
} Queue_node;

// queue used for algorithms and to track positions to re-draw path of all algorithms
typedef struct queue_t
{
    int count;
    Queue_node *front;
    Queue_node *rear;
} Queue;

typedef struct heap_node_t
{
    Position tile_position;
    int f_cost;
} Heap_node;

typedef struct heap_t
{
    Heap_node *array;
    int count;
    int capacity;
} Heap;

// Stack to track 2D-position {X, Y}
typedef struct stack_t
{
    int top;
    int capacity;
    Position *array;
} Stack;

// globals
int global_rows = 80;
int global_colums = 80;
int tile_width = 9;
int tile_height = 9;
Position start_node;
Position end_node;
Queue *search_path;
Queue *final_path;
Position draw_position;
Font font;
Tile **tiles;
bool draw_instant = false;
double timedt = 0.0;
char time_buffer[50];
double timeout = 0;
Rectangle button_reset;
Rectangle button_draw_instant_toggle;
Color toggle_color;
Rectangle algo_button_background;
Rectangle button_dijkstra;
Rectangle button_aStar;
Rectangle button_bfs;
Rectangle button_dfs;
Rectangle button_size_plus_ten;
Rectangle button_size_minus_ten;
Rectangle mazebutton_background;
Rectangle button_N_maze;
Rectangle button_wall;

// Update and Draw one frame for WebAssembly
void UpdateDrawFrame(void);

// UI-functions
void left_mouse_is_down();
void right_mouse_is_down();
void init_buttons();
void draw_tiles();
void draw_buttons();
void realloc_tiles();
void init_tiles(bool clearWalls);
void de_init_tiles();
void set_search_path_tile();
void set_final_path_tile();

// Queue functions
bool queue_is_empty(Queue *q);
void queue_init(Queue *q);
void queue_enqueue(Queue *q, Position pos);
Position queue_dequeue(Queue *q);

// algorythm functions
void dijkstra();
void aStar();
void breath_first_search();
void depth_first_search();

int main(void)
{

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "AlgoSim");
    SetTargetFPS(TARGET_FPS);
    start_node = (Position){4, 4};
    end_node = (Position){global_rows - 4, global_colums - 4};
    realloc_tiles();
    init_tiles(true);
    init_buttons();
    search_path = malloc(sizeof(Queue));
    final_path = malloc(sizeof(Queue));
    queue_init(search_path);
    queue_init(final_path);
    toggle_color = GREEN;
    char* dir = GetWorkingDirectory();
    font = LoadFontEx(FONT_RESOURCE, 60, 0, 0);
    // Main game loop

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif
    // De-Initialization
    CloseWindow();
    de_init_tiles();
    free(search_path);
    return 0;
}

void UpdateDrawFrame(void)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        left_mouse_is_down();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        right_mouse_is_down();
    }

    // prepare color for searched path of a algorythm
    if (!queue_is_empty(search_path))
    {
        set_search_path_tile();
    }

    // prepare color for final path of a algorythm
    if (queue_is_empty(search_path) && !queue_is_empty(final_path))
    {
        set_final_path_tile();
    }

    // Rendering
    BeginDrawing();
    {
        ClearBackground(WHITE);

        // draws all vertices
        draw_tiles();

        // draw elapsed Time
          
        DrawTextEx(font, time_buffer, (Vector2) {300, 730}, 40, 2, BLACK);
        draw_buttons();
        DrawFPS(SCREENWIDTH - 80, SCREENHEIGHT - 80);
    }
    EndDrawing();
}

void init_buttons()
{
    // button Reset
    button_reset.height = BUTTONHEIGHT;
    button_reset.width = BUTTONWIDTH;
    button_reset.x = SCREENWIDTH - 350;
    button_reset.y = SCREENHEIGHT - 100;

    // Button Start Dijkstra
    button_dijkstra.height = BUTTONHEIGHT;
    button_dijkstra.width = BUTTONWIDTH;
    button_dijkstra.x = SCREENWIDTH - 350;
    button_dijkstra.y = SCREENHEIGHT - 250;

    // Button: Start AStar
    button_aStar.height = BUTTONHEIGHT;
    button_aStar.width = BUTTONWIDTH;
    button_aStar.x = SCREENWIDTH - 350;
    button_aStar.y = SCREENHEIGHT - 360;

    // Button: Start Breadth First Search
    button_bfs.height = BUTTONHEIGHT;
    button_bfs.width = BUTTONWIDTH;
    button_bfs.x = SCREENWIDTH - 350;
    button_bfs.y = SCREENHEIGHT - 470;

    // Button: Start Depth First Search
    button_dfs.height = BUTTONHEIGHT;
    button_dfs.width = BUTTONWIDTH;
    button_dfs.x = SCREENWIDTH - 350;
    button_dfs.y = SCREENHEIGHT - 580;

    // Button: Increase rows and colums by 10
    button_size_plus_ten.height = BUTTONHEIGHT;
    button_size_plus_ten.width = BUTTONWIDTH;
    button_size_plus_ten.x = 10;
    button_size_plus_ten.y = 730;

    // Button: Decrease rows and colums by 10
    button_size_minus_ten.height = BUTTONHEIGHT;
    button_size_minus_ten.width = BUTTONWIDTH;
    button_size_minus_ten.x = 10;
    button_size_minus_ten.y = 730 + 10 + BUTTONHEIGHT;

    // Background for Algorithm buttons
    algo_button_background.height = 520;
    algo_button_background.width = 320;
    algo_button_background.x = 730;
    algo_button_background.y = 190;

    // Background for maze buttonstimedt
    mazebutton_background.height = 100;
    mazebutton_background.width = 320;
    mazebutton_background.x = 730;
    mazebutton_background.y = 10;

    // Button: draw N shaped walls
    button_N_maze.height = BUTTONHEIGHT;
    button_N_maze.width = BUTTONWIDTH;
    button_N_maze.x = SCREENWIDTH - 350;
    button_N_maze.y = BUTTONHEIGHT + 20;

    // Button: draw wall in middle
    button_wall.height = BUTTONHEIGHT;
    button_wall.width = BUTTONWIDTH;
    button_wall.x = SCREENWIDTH - 200;
    button_wall.y = BUTTONHEIGHT + 20;

    // toggle draw animation for search path
    button_draw_instant_toggle.height = BUTTONHEIGHT;
    button_draw_instant_toggle.width = BUTTONWIDTH;
    button_draw_instant_toggle.x = 10;
    button_draw_instant_toggle.y = 850;
}

// re-inits all tiles (not walls) and starts given algorithm. Time taken of algorithm is also saved in time_buffer as string
void start_algorithm(void (*f)())
{

    // code to measure Time complexity:

    // int size = 10;
    // for (int i = 100; i < 500001; i = i * 2)
    //{
    //     de_init_tiles();
    //     size = (int)sqrt(i);
    //     global_rows = size;
    //     global_colums = size;
    //     tile_height = 1;
    //     tile_width = 1;
    //     end_node = (Position){global_rows - 4, global_colums -4};
    //     realloc_tiles();
    //     init_tiles(true);
    //
    //     for (size_t j = 1; j < global_rows; j++){
    //         tiles[j][global_colums/2].weight = WALLWEIGHT;
    //         tiles[j][global_colums/2].color = RED;
    //     }
    //
    //     queue_init(search_path);
    //     queue_init(final_path);
    //     de_init_tiles();
    //     init_tiles(false);
    //     timedt = GetTime();
    //     f();
    //     timedt = GetTime() - timedt;
    //     sprintf(time_buffer, "Elapsed Time:\n %.10f [ms]", timedt * 1000);
    //
    //     printf("%d;%.10f\n", i, timedt * 1000);
    // }

    queue_init(search_path);
    queue_init(final_path);
    de_init_tiles();
    init_tiles(false);
    timedt = GetTime();
    f();
    timedt = GetTime() - timedt;
    sprintf(time_buffer, "Elapsed Time:\n %.10f [ms]", timedt * 1000);
}

// handle left mouse click
void left_mouse_is_down()
{
    Vector2 pos = GetMousePosition();

    uint8_t rowIndex = (uint8_t)(pos.y / tile_height);
    uint8_t colIndex = (uint8_t)(pos.x / tile_width);
    if ((rowIndex < global_rows) && (colIndex < global_colums))
    {
        tiles[rowIndex][colIndex].color = RED;
        tiles[rowIndex][colIndex].weight = WALLWEIGHT;
        if (rowIndex > 0 && colIndex > 0 && rowIndex < global_rows - 1 && colIndex < global_colums - 1)
        {
            tiles[rowIndex + 1][colIndex].color = RED;
            tiles[rowIndex + 1][colIndex].weight = WALLWEIGHT;
            tiles[rowIndex][colIndex + 1].color = RED;
            tiles[rowIndex][colIndex + 1].weight = WALLWEIGHT;
        }
    }

    if (GetTime() - timeout >= 0.5f)
    {
        timeout = GetTime();
        // reset button click
        if (CheckCollisionPointRec(pos, button_reset))
        {
            queue_init(search_path);
            queue_init(final_path);
            de_init_tiles();
            init_tiles(true);
        }
        // dijkstra button click
        else if (CheckCollisionPointRec(pos, button_dijkstra))
        {
            start_algorithm(dijkstra);
        }
        // aStar button click
        else if (CheckCollisionPointRec(pos, button_aStar))
        {
            start_algorithm(aStar);
        }
        // Breadth First Search button click
        else if (CheckCollisionPointRec(pos, button_bfs))
        {
            start_algorithm(breath_first_search);
        }
        // Depth First Search button click
        else if (CheckCollisionPointRec(pos, button_dfs))
        {
            start_algorithm(depth_first_search);
        }
        else if (CheckCollisionPointRec(pos, button_size_plus_ten))
        {
            de_init_tiles();
            global_rows = global_rows + 10;
            global_colums = global_colums + 10;
            tile_height = 720 / global_rows;
            tile_width = 720 / global_colums;
            end_node = (Position){global_rows - 4, global_colums - 4};
            realloc_tiles();
            init_tiles(true);
        }
        else if (CheckCollisionPointRec(pos, button_size_minus_ten))
        {
            if (global_rows > 10)
            {
                de_init_tiles();
                global_rows = global_rows - 10;
                global_colums = global_colums - 10;
                tile_height = 720 / global_rows;
                tile_width = 720 / global_colums;
                end_node = (Position){global_rows - 4, global_colums - 4};
                realloc_tiles();
                init_tiles(true);
            }
        }
        else if (CheckCollisionPointRec(pos, button_draw_instant_toggle))
        {
            draw_instant = !draw_instant;
            if (!draw_instant)
            {
                toggle_color = GREEN;
            }
            else
            {
                toggle_color = RED;
            }
        }
        else if (CheckCollisionPointRec(pos, button_N_maze))
        {
            de_init_tiles();
            init_tiles(false);
            Vector2 pos1, pos2, pos3, pos4, tile;
            pos1 = (Vector2){0, 720};
            pos2 = (Vector2){720 / 4, tile_height * 2};
            pos3 = (Vector2){(3 * 700) / 4, 720};
            pos4 = (Vector2){720 - (tile_width * 5), tile_height * 5};

            for (size_t i = 0; i < global_rows; i++)
            {
                for (size_t j = 0; j < global_rows; j++)
                {
                    tile = (Vector2){tiles[i][j].rectangle.x, tiles[i][j].rectangle.y};
                    if (CheckCollisionPointLine(tile, pos1, pos2, 15) || CheckCollisionPointLine(tile, pos2, pos3, 15) || CheckCollisionPointLine(tile, pos3, pos4, 15))
                    {
                        tiles[i][j].weight = WALLWEIGHT;
                        tiles[i][j].color = RED;
                    }
                }
            }
            // ensure that there is a free way on the right and top edge
            for (size_t i = 0; i < global_rows; i++)
            {
                tiles[0][i].weight = 1;
                if (i % 2)
                {
                    tiles[i][global_colums - 1].color = SKYBLUE;
                }
                else
                {
                    tiles[i][global_colums - 1].color = BLUE;
                }
            }
            for (size_t i = 0; i < global_colums; i++)
            {
                tiles[i][global_colums - 1].weight = 1;
                if (i % 2)
                {
                    tiles[i][global_colums - 1].color = SKYBLUE;
                }
                else
                {
                    tiles[i][global_colums - 1].color = BLUE;
                }
            }
        }
        else if (CheckCollisionPointRec(pos, button_wall))
        {
            de_init_tiles();
            init_tiles(false);
            for (size_t j = 1; j < global_rows; j++)
            {
                tiles[j][global_colums / 2].weight = WALLWEIGHT;
                tiles[j][global_colums / 2].color = RED;
            }
        }
    }
}

void right_mouse_is_down()
{
    Vector2 pos = GetMousePosition();

    uint8_t rowIndex = (uint8_t)(pos.y / tile_height);
    uint8_t colIndex = (uint8_t)(pos.x / tile_width);
    if ((rowIndex < global_rows) && (colIndex < global_colums))
    {
        tiles[rowIndex][colIndex].color = GRAY;
        tiles[rowIndex][colIndex].weight = 20;
        if (rowIndex > 0 && colIndex > 0 && rowIndex < global_rows - 1 && colIndex < global_colums - 1)
        {
            tiles[rowIndex + 1][colIndex].color = GRAY;
            tiles[rowIndex + 1][colIndex].weight = 20;
            tiles[rowIndex][colIndex + 1].color = GRAY;
            tiles[rowIndex][colIndex + 1].weight = 20;
        }
    }
}

// init/reset a queue
void queue_init(Queue *queue)
{
    queue->count = 0;
    queue->front = NULL;
    queue->rear = NULL;
}

// check if queue is empty
bool queue_is_empty(Queue *queue)
{
    if (queue->count == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// enqueues a new Position
void queue_enqueue(Queue *queue, Position pos)
{
    Queue_node *tmp;
    tmp = malloc(sizeof(Queue_node));
    tmp->tile_position = pos;
    tmp->next = NULL;
    if (queue_is_empty(queue) == false)
    {
        queue->rear->next = tmp;
        queue->rear = tmp;
    }
    else
    {
        queue->front = queue->rear = tmp;
    }
    queue->count++;
}

// dequeues from the front of the Queue (FIFO)
Position queue_dequeue(Queue *queue)
{
    Queue_node *tmp;
    if (queue->count != 0)
    {
        Position n = queue->front->tile_position;
        tmp = queue->front;
        queue->front = queue->front->next;
        queue->count--;
        free(tmp);
        return n;
    }
    else
    {
        return (Position){-1, -1};
    }
}

// create stack with capacity as array size
Stack *stack_create(int capacity)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (Position *)malloc(stack->capacity * sizeof(Position));
    return stack;
}

bool stack_is_full(Stack *stack)
{
    if (stack->top == stack->capacity - 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool stack_is_empty(Stack *stack)
{
    if (stack->top == -1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void stack_push(Stack *stack, Position new_pos)
{
    if (stack_is_full(stack))
    {
        return;
    }

    stack->array[++stack->top] = new_pos;
}

Position stack_pop(Stack *stack)
{
    if (stack_is_empty(stack))
    {
        return (Position){-1, -1};
    }
    return stack->array[stack->top--];
}

Heap *CreateMinHeap(int capacity)
{
    Heap *h = (Heap *)malloc(sizeof(Heap));

    h->count = 0;                                                 // starts with 0 elements
    h->capacity = capacity;                                       // max elements
    h->array = (Heap_node *)malloc(capacity * sizeof(Heap_node)); // allocate size for maximum possible Heap size
    return h;
}

// heapify recursively from bottom to the top of the given Heap
void heapify_bottom_to_top(Heap *h, int index)
{
    Heap_node temp;
    int parent_index = (index - 1) / 2; // gets the parent index (decimals get truncated)

    // compare f cost with parent and swap nodes recursively if necessary
    if (h->array[parent_index].f_cost > h->array[index].f_cost)
    {
        temp = h->array[parent_index];
        h->array[parent_index] = h->array[index];
        h->array[index] = temp;
        heapify_bottom_to_top(h, parent_index);
    }
}

// heapify recursively from top to the bottom of the given Heap
void heapify_top_to_bottom(Heap *h, int parent_index)
{
    int left = parent_index * 2 + 1;
    int right = parent_index * 2 + 2;
    int min;
    Heap_node temp;

    // check for end
    if (left >= h->count || left < 0)
    {
        left = -1;
    }
    if (right >= h->count || right < 0)
    {
        right = -1;
    }

    if (left != -1 && h->array[left].f_cost < h->array[parent_index].f_cost)
    {
        min = left;
    }
    else
    {
        min = parent_index;
    }

    if (right != -1 && h->array[right].f_cost < h->array[min].f_cost)
    {
        min = right;
    }

    if (min != parent_index)
    {
        temp = h->array[min];
        h->array[min] = h->array[parent_index];
        h->array[parent_index] = temp;

        heapify_top_to_bottom(h, min);
    }
}

// add element to heap
void insert_heap(Heap *h, Heap_node node)
{
    if (h->count < h->capacity)
    {
        h->array[h->count] = node;
        heapify_bottom_to_top(h, h->count);
        h->count++;
    }
}

// returns top element of heap
Heap_node popMin_heap(Heap *h)
{
    Heap_node pop;
    if (h->count == 0)
    {
        printf("\n__Heap is Empty__\n");
    }
    // first node is the minimum, because we use a MinHeap
    pop = h->array[0];
    h->array[0] = h->array[h->count - 1];
    h->count--;
    heapify_top_to_bottom(h, 0);
    return pop;
}

// reallocates space for the 2D array of tiles
void realloc_tiles()
{
    free(tiles);
    tiles = (Tile **)malloc(global_rows * sizeof(Tile *));
    for (size_t i = 0; i < global_rows; i++)
    {
        tiles[i] = (Tile *)malloc(global_colums * sizeof(Tile));
    }
}

// Draw each Tile as a rectangle on the canvas
void draw_tiles()
{
    for (size_t row = 0; row < global_rows; row++)
    {
        for (size_t column = 0; column < global_colums; column++)
        {
            DrawRectangleRec(tiles[row][column].rectangle, tiles[row][column].color);
        }
    }
}

// Draw all buttons with text
void draw_buttons()
{
    DrawRectangleRec(button_reset, RED); // Reset tiles
    DrawTextEx(font, "Stop and\nReset", (Vector2) {button_reset.x, button_reset.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(algo_button_background, LIGHTGRAY); // Background for algorithm buttons
    DrawTextEx(font, "Algorithms", (Vector2) {algo_button_background.x + 20, algo_button_background.y}, 40, 2, DARKGREEN);

    DrawRectangleRec(button_draw_instant_toggle, toggle_color); // Toggle search path animation
    DrawTextEx(font, "Path animation", (Vector2) {button_draw_instant_toggle.x, button_draw_instant_toggle.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(button_dijkstra, YELLOW); // Start Dijkstra
    DrawTextEx(font,  "Dijkstra", (Vector2) {button_dijkstra.x, button_dijkstra.y}, FONTSIZE + 10, 2, BLACK);

    DrawRectangleRec(button_aStar, GREEN); // Start A Star
    DrawTextEx(font,  "AStar", (Vector2) {button_aStar.x, button_aStar.y}, FONTSIZE + 10, 2, BLACK);

    DrawRectangleRec(button_bfs, MAROON); // Start BFS
    DrawTextEx(font,  "Breadth\nFirst Search", (Vector2) {button_bfs.x, button_bfs.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(button_dfs, GRAY); // Start DFS
    DrawTextEx(font,  "Depth\nFirst Search", (Vector2) {button_dfs.x, button_dfs.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(button_size_plus_ten, GRAY); // increase rows and colums by 10
    DrawTextEx(font,  "Rows/Colums +10", (Vector2) {button_size_plus_ten.x, button_size_plus_ten.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(button_size_minus_ten, GRAY); // decrease rows and colums by 10
    DrawTextEx(font,  "Rows/Colums -10", (Vector2) {button_size_minus_ten.x, button_size_minus_ten.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(mazebutton_background, BEIGE); // background for maze buttons
    DrawTextEx(font,  "Predefined mazes", (Vector2) {mazebutton_background.x + 20, mazebutton_background.y}, 25, 2, BLACK);

    DrawRectangleRec(button_N_maze, GRAY); // draw N shaped maze
    DrawTextEx(font,  "N-Maze", (Vector2) {button_N_maze.x, button_N_maze.y}, FONTSIZE, 2, BLACK);

    DrawRectangleRec(button_wall, GRAY); // draw N shaped maze
    DrawTextEx(font,  "Middle wall", (Vector2) {button_wall.x, button_wall.y}, FONTSIZE, 2, BLACK);
}

// sets selected neighbours to a Tile
void set_neighbours(Tile *my_tile, int count, bool north, bool east, bool south, bool west)
{
    my_tile->neighbours = malloc(sizeof(Position) * count);
    my_tile->neighbour_count = count;
    Position pos = my_tile->tile_position;
    int index = 0;

    if (north)
    {
        my_tile->neighbours[index].row = pos.row - 1;
        my_tile->neighbours[index].column = pos.column;
        index++;
    }
    if (east)
    {
        my_tile->neighbours[index].row = pos.row;
        my_tile->neighbours[index].column = pos.column + 1;
        index++;
    }
    if (south)
    {
        my_tile->neighbours[index].row = pos.row + 1;
        my_tile->neighbours[index].column = pos.column;
        index++;
    }
    if (west)
    {
        my_tile->neighbours[index].row = pos.row;
        my_tile->neighbours[index].column = pos.column - 1;
        index++;
    }
}

void init_tiles(bool clearWalls)
{
    Tile *temp;
    for (size_t row = 0; row < global_rows; row++)
    {
        for (size_t column = 0; column < global_colums; column++)
        {
            temp = &tiles[row][column];
            temp->tile_position.row = row;
            temp->tile_position.column = column;
            temp->rectangle.height = tile_height;
            temp->rectangle.width = tile_width;
            temp->rectangle.x = column * tile_width;
            temp->rectangle.y = row * tile_height;
            temp->f = INT_MAX;
            temp->g = INT_MAX;
            temp->predesessor = (Position){-1, -1};

            // "chessboard"-coloring
            if ((row + column) % 2)
            {
                temp->color = BLUE;
            }
            else if ((row == start_node.row) && (column == start_node.column))
            {
                temp->color = PURPLE;
            }
            else if ((row == end_node.row) && (column == end_node.column))
            {
                temp->color = GOLD;
            }
            else
            {
                temp->color = SKYBLUE;
            }

            if (!clearWalls && (temp->weight == WALLWEIGHT || temp->weight == 20))
            {
                if (temp->weight == WALLWEIGHT)
                {
                    temp->color = RED;
                }
                else
                {
                    temp->color = GRAY;
                }
            }
            else
            {
                temp->weight = 1;
            }
            temp->inOpenList = false;
            temp->finalized = false;

            // tiles that are not in a corner or at the edge
            if (row > 0 && row < global_rows - 1 && column > 0 && column < global_colums - 1)
            {
                set_neighbours(temp, 4, true, true, true, true);
            }
            // top left Tile
            else if (row == 0 && column == 0)
            {
                set_neighbours(temp, 2, false, true, true, false);
            }
            // top right Tile
            else if (row == 0 && column == global_colums - 1)
            {
                set_neighbours(temp, 2, false, false, true, true);
            }
            // bottom left Tile
            else if (row == global_rows - 1 && column == 0)
            {
                set_neighbours(temp, 2, true, true, false, false);
            }
            // bottom right Tile
            else if (row == global_rows - 1 && column == global_colums - 1)
            {
                set_neighbours(temp, 2, true, false, false, true);
            }
            // top edge
            else if (row == 0 && column > 0 && column < global_colums - 1)
            {
                set_neighbours(temp, 3, false, true, true, true);
            }
            // right edge
            else if (column == global_colums - 1 && row > 0 && row < global_rows - 1)
            {
                set_neighbours(temp, 3, true, false, true, true);
            }
            // bottom edge
            else if (row == global_rows - 1 && column > 0 && column < global_colums - 1)
            {
                set_neighbours(temp, 3, true, true, false, true);
            }
            // left edge
            else if (column == 0 && row > 0 && row < global_rows - 1)
            {
                set_neighbours(temp, 3, true, true, true, false);
            }
        }
    }
}

// rule of thumb: every malloc needs a free. Thus the de_init
void de_init_tiles()
{
    for (size_t row = 0; row < global_rows; row++)
    {
        for (size_t column = 0; column < global_colums; column++)
        {
            free(tiles[row][column].neighbours);
        }
    }
}

// sets color of a dequeued Tile (from the searched path) to green
void set_search_path_tile()
{
    if (draw_instant)
    {
        while (!queue_is_empty(search_path))
        {
            draw_position = queue_dequeue(search_path);
            if (draw_position.row != -1 && !(start_node.row == draw_position.row && start_node.column == draw_position.column))
            {
                tiles[draw_position.row][draw_position.column].color = GREEN;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < DRAWSPEED; i++)
        {
            draw_position = queue_dequeue(search_path);
            if (draw_position.row != -1 && !(start_node.row == draw_position.row && start_node.column == draw_position.column))
            {
                tiles[draw_position.row][draw_position.column].color = GREEN;
            }
        }
    }
}

// sets color of a dequeued Tile (from the finished path) to black
void set_final_path_tile()
{
    for (size_t i = 0; i < DRAWSPEED; i++)
    {
        draw_position = queue_dequeue(final_path);
        if (draw_position.row != -1)
        {
            tiles[draw_position.row][draw_position.column].color = BLACK;
        }
    }
}

// check if distance in all tiles are already calculated
bool dijkstra_all_tiles_finalized()
{
    for (size_t row = 0; row < global_rows; row++)
    {
        for (size_t column = 0; column < global_colums; column++)
        {
            if (tiles[row][column].finalized == false)
            {
                return false;
            }
        }
    }
    return true;
}

// calculate distances for each neighbour / update reach_cost
void dijkstra_update_distance(Position current_u)
{
    int adjacent_row;
    int adjacent_column;
    int tmp_reach_cost;
    Tile *current_tile = &tiles[current_u.row][current_u.column];
    Tile *adjacennt_tile;

    // for each possible neighbour
    for (size_t i = 0; i < current_tile->neighbour_count; i++)
    {
        adjacent_row = current_tile->neighbours[i].row;
        adjacent_column = current_tile->neighbours[i].column;
        adjacennt_tile = &tiles[adjacent_row][adjacent_column];

        // if not already finalized and not boundary of grid
        if ((adjacennt_tile->finalized == false) && (adjacent_row != -1) && (adjacent_column != -1) && (adjacent_row < global_rows) && (adjacent_column < global_colums))
        {
            tmp_reach_cost = current_tile->reach_cost + adjacennt_tile->weight;
            if (tmp_reach_cost < adjacennt_tile->reach_cost)
            {
                adjacennt_tile->reach_cost = tmp_reach_cost;
                adjacennt_tile->predesessor = current_u;
            }
        }
    }
}

// orientated on following explanation : https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
void dijkstra()
{
    // STEP: Init all distances to INIFINTE. Assign distatnce for source vertex to 0
    for (size_t row = 0; row < global_rows; row++)
    {
        for (size_t column = 0; column < global_colums; column++)
        {
            tiles[row][column].finalized = false; // set to => "shortest path not found yet"
            if ((row == start_node.row) && (column == start_node.column))
            {
                tiles[row][column].reach_cost = 0;
            }
            else
            {
                tiles[row][column].reach_cost = INT_MAX;
            }
        }
    }

    bool finished = false;
    int min_dist = INT_MAX;
    Position current_u;
    Position shortest;

    // while still tiles in array, that are not finalized
    while (!finished)
    {
        min_dist = INT_MAX;

        // STEP: search Tile with minimum distance, which is not finalized
        for (size_t row = 0; row < global_rows; row++)
        {
            for (size_t column = 0; column < global_colums; column++)
            {
                if (!tiles[row][column].finalized)
                {
                    if (tiles[row][column].reach_cost < min_dist)
                    {
                        min_dist = tiles[row][column].reach_cost;
                        current_u = (Position){row, column};
                    }
                }
            }
        }

        // current_u has minumum distance, so it is finalized
        tiles[current_u.row][current_u.column].finalized = true;
        // calculate neighbour distances from current_u
        dijkstra_update_distance(current_u);
        // add to Queue for new finalized Tile
        queue_enqueue(search_path, current_u);
        // STEP: All tiles finalized?

        finished = dijkstra_all_tiles_finalized();

        if ((current_u.row == end_node.row) && (current_u.column == end_node.column))
        {

            while ((current_u.row != start_node.row) || (current_u.column != start_node.column))
            {
                shortest = tiles[current_u.row][current_u.column].predesessor;
                queue_enqueue(final_path, current_u);
                current_u = shortest;
            }
            finished = true;
        }
    }
}

// Euclidean distance, but to the power of 2 (avoiding the square root). returns h(n)^2 = (from.x - to.x)^2 + (from.y -to.y)^2
int heuristic(Position from, Position to)
{
    return (from.row - to.row) * (from.row - to.row) + (from.column - to.column) * (from.column - to.column);
}

// A-Star algorythm allows for 4-way directional movement (north,east,south,west) . See also: https://www.growingwiththeweb.com/2012/06/a-pathfinding-algorithm.html#:~:text=A%20pathfinding%20algorithm%20takes%20a,on%20any%20type%20of%20graph.
void aStar()
{
    int maxHeapCapacity = global_rows * global_colums;
    int cur_row;
    int cur_col;
    Tile *cur_tile;
    int neighbour_row;
    int neighbour_col;
    Tile *neighbour_tile;
    Heap *openList = CreateMinHeap(maxHeapCapacity); // init Heap
    Heap_node tmp_heap_node;                         // create start node
    Heap_node current;
    tmp_heap_node.tile_position = start_node;                                // init start node Position
    tmp_heap_node.f_cost = heuristic(tmp_heap_node.tile_position, end_node); // init start node f cost
    insert_heap(openList, tmp_heap_node);                                    // add start node to Heap

    // while openList not empty "openList->count > 0"
    while (openList->count > 0)
    {

        current = popMin_heap(openList);
        cur_row = current.tile_position.row;
        cur_col = current.tile_position.column;
        cur_tile = &tiles[cur_row][cur_col];
        queue_enqueue(search_path, current.tile_position);
        // stop if current node is the goal and construct the final path
        if ((cur_row == end_node.row) && (cur_col == end_node.column))
        {
            while (!((cur_tile->predesessor.row == start_node.row) && (cur_tile->predesessor.column == start_node.column)))
            {
                queue_enqueue(final_path, cur_tile->predesessor);
                cur_tile = &tiles[cur_tile->predesessor.row][cur_tile->predesessor.column];
            }
            break;
        }

        cur_tile->finalized = true; // ,,adding to closed list" by saying that this Tile is finalized
        cur_tile->inOpenList = false;

        // for each neighbour of current node (up to 4 neighbours, because we only check north, east, south, west)
        for (size_t i = 0; i < cur_tile->neighbour_count; i++)
        {
            neighbour_row = cur_tile->neighbours[i].row;
            neighbour_col = cur_tile->neighbours[i].column;
            neighbour_tile = &tiles[neighbour_row][neighbour_col];

            // if neighbour not already finalized
            if (!neighbour_tile->finalized)
            {
                neighbour_tile->g = cur_tile->g + neighbour_tile->weight; // g is the incremental cost of the weights from the start to this Tile
                neighbour_tile->f = neighbour_tile->g + heuristic(neighbour_tile->tile_position, end_node);
                neighbour_tile->predesessor = cur_tile->tile_position;
                // insert all neighbours into the openList(Heap) (if not already inserted)
                if (!neighbour_tile->inOpenList)
                {
                    neighbour_tile->inOpenList = true;
                    // printf("%d   : %d\n", tmp_heap_node.tile_position.row, tmp_heap_node.tile_position.column);
                    tmp_heap_node.f_cost = neighbour_tile->f;
                    tmp_heap_node.tile_position = neighbour_tile->tile_position;
                    insert_heap(openList, tmp_heap_node);
                }
            }
        }
    }
    free(openList);

}

void bfs_dfs_construct_finished_path()
{
    Position cur_pos = end_node;
    while (cur_pos.row != -1)
    {
        queue_enqueue(final_path, cur_pos);
        cur_pos = tiles[cur_pos.row][cur_pos.column].predesessor;
    }
}

// Breath First Search algorythm. See also : https://medium.com/@urna.hybesis/pathfinding-algorithms-the-four-pillars-1ebad85d4c6b
void breath_first_search()
{
    Queue *bfs_queue = malloc(sizeof(Queue));
    queue_init(bfs_queue);
    Position curPos;
    Tile *cur_tile;
    Tile *neighbour_tile;

    cur_tile = &tiles[start_node.row][start_node.column];
    cur_tile->finalized = true;

    queue_enqueue(bfs_queue, start_node);
    // while there is a node to handle in the Queue
    while (!queue_is_empty(bfs_queue))
    {
        curPos = queue_dequeue(bfs_queue);
        // enqueue for the visualization
        queue_enqueue(search_path, curPos);

        cur_tile = &tiles[curPos.row][curPos.column];

        if ((curPos.row == end_node.row) && (curPos.column == end_node.column))
        {
            break;
        }

        for (size_t i = 0; i < cur_tile->neighbour_count; i++)
        {
            curPos.row = cur_tile->neighbours[i].row;
            curPos.column = cur_tile->neighbours[i].column;
            neighbour_tile = &tiles[curPos.row][curPos.column];

            if (!neighbour_tile->finalized && neighbour_tile->weight != WALLWEIGHT)
            {
                neighbour_tile->finalized = true;
                neighbour_tile->predesessor.row = cur_tile->tile_position.row;
                neighbour_tile->predesessor.column = cur_tile->tile_position.column;
                queue_enqueue(bfs_queue, curPos);
            }
        }
    }

    bfs_dfs_construct_finished_path(end_node);
    free(bfs_queue);
}

// Depth First Search algorythm. See also : https://medium.com/@urna.hybesis/pathfinding-algorithms-the-four-pillars-1ebad85d4c6b
void depth_first_search()
{
    Stack *dfs_stack = stack_create(global_rows * global_colums);
    Position curPos;
    Tile *cur_tile;
    Tile *neighbour_tile;
    stack_push(dfs_stack, start_node);
    cur_tile = &tiles[start_node.row][start_node.column];
    cur_tile->finalized = true;

    while (!stack_is_empty(dfs_stack))
    {

        curPos = stack_pop(dfs_stack);
        // enqueue for the visualization
        queue_enqueue(search_path, curPos);

        cur_tile = &tiles[curPos.row][curPos.column];
        if ((curPos.row == end_node.row) && (curPos.column == end_node.column))
        {
            break;
        }

        for (size_t i = 0; i < cur_tile->neighbour_count; i++)
        {
            curPos.row = cur_tile->neighbours[i].row;
            curPos.column = cur_tile->neighbours[i].column;
            neighbour_tile = &tiles[curPos.row][curPos.column];

            if (!neighbour_tile->finalized && neighbour_tile->weight != WALLWEIGHT)
            {
                neighbour_tile->finalized = true;
                neighbour_tile->predesessor.row = cur_tile->tile_position.row;
                neighbour_tile->predesessor.column = cur_tile->tile_position.column;

                stack_push(dfs_stack, curPos);
            }
        }
    }
    free(dfs_stack);
    bfs_dfs_construct_finished_path(end_node);
}

// TODO: not used right now.
// Sets random tiles as Walls besides the start and end node
void maze_random(int percent)
{
    int tile_count = global_rows * global_colums;
    int tiles_to_cover = (int)(tile_count / 100) * percent;
    int ran_row = 0;
    int ran_col = 0;

    for (int i = 0; i < tiles_to_cover; i++)
    {
        ran_row = GetRandomValue(0, global_rows - 1);
        ran_col = GetRandomValue(0, global_colums - 1);
        tiles[ran_row][ran_col].weight = WALLWEIGHT;
        tiles[ran_row][ran_col].color = RED;
    }
    tiles[start_node.row][start_node.column].weight = 1;
    tiles[end_node.row][end_node.column].weight = 1;
}
