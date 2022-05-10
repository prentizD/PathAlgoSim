#include "include\raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#define SCREENWIDTH 1000
#define SCREENHEIGHT 800
#define ROWS 60
#define COLUMS 60
#define NODEWIDTH 10
#define NODEHEIGHT 10
#define TARGET_FPS 60
#define START_X 10
#define START_Y 10
#define PATHDRAWSPEED 10       // TARGET_FPS * PATHDRAWSPEED = drawn tiles (that are in a queue) per seconds
#define WEIGHTS 1

typedef struct tile_position_t
{
    int row;
    int column;
} tile_position_t;

typedef struct tile_t
{
    tile_position_t tile_position;
    Rectangle rectangle;
    Color color;
    int weight;
    int reach_cost;
    bool finalized;
    tile_position_t *neighbours;    // not a valid neighbour if row or column is -1
    tile_position_t predecessor;
} tile_t;

typedef struct node_t
{
    tile_position_t tile_position;
    struct node_t* next;
} node_t;

typedef struct queue_t
{
    int count;
    node_t *front;
    node_t *rear;
} queue_t;

static queue_t *algorythm_steps;
static tile_position_t draw_position;
static tile_t tiles[ROWS][COLUMS];
static bool queue_is_empty;


bool is_queue_empty(queue_t *q);
void init_queue(queue_t *q);
void enqueue(queue_t *q, tile_position_t pos);
tile_position_t dequeue(queue_t *q);

void draw_tiles(void);
void init_tiles(void);
void de_init_tiles(void);
void set_tile_path(void);

void dijkstra(tile_position_t startnode);

int main(void)
{
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Test");
    SetTargetFPS(TARGET_FPS);                   // Set our game to run at 60 frames-per-second
    init_tiles();
    algorythm_steps = malloc(sizeof(queue_t));
    init_queue(algorythm_steps);
    tile_position_t start_node = {START_X, START_Y};
    //dijkstra(start_node);

    Rectangle button_reset;
    button_reset.height = 60;
    button_reset.width = 120;
    button_reset.x = SCREENWIDTH - 300;
    button_reset.y = SCREENHEIGHT - 80;

    Rectangle button_dijkstra;
    button_dijkstra.height = 60;
    button_dijkstra.width = 120;
    button_dijkstra.x = SCREENWIDTH - 600;
    button_dijkstra.y = SCREENHEIGHT - 80;

    double time = 0;
    char time_buffer[50];
    double timeout = 0;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {  
        // loop logic
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 pos = GetMousePosition();
            uint8_t rowIndex = (uint8_t) (pos.y/NODEHEIGHT);
            uint8_t colIndex = (uint8_t) (pos.x/NODEWIDTH);
            if((rowIndex < ROWS) && (colIndex < COLUMS))
            {
                tiles[rowIndex][colIndex].color = RED;
                tiles[rowIndex][colIndex].weight = 1000;
                if(rowIndex > 0 && colIndex >0)
                {
                    tiles[rowIndex+1][colIndex].color = RED;
                    tiles[rowIndex+1][colIndex].weight = 1000;               
                    tiles[rowIndex][colIndex+1].color = RED;
                    tiles[rowIndex][colIndex+1].weight = 1000;
                }
            }
            if(GetTime()-timeout >= 0.5f)
            {
                timeout = GetTime();
                if(CheckCollisionPointRec(pos, button_dijkstra))
                { 
                    
                    time = GetTime();
                    dijkstra(start_node);
                    time = GetTime() - time;
                    sprintf(time_buffer, "%.10f [ms]", time * 1000);
                }
                if(CheckCollisionPointRec(pos, button_reset))
                {
                    init_queue(algorythm_steps);
                    de_init_tiles();
                    init_tiles();
                }
            }
        }
        
        if(!queue_is_empty)
        {
            set_tile_path();
        }
        // drawing
        BeginDrawing();
        {
            ClearBackground(WHITE);
            draw_tiles();
            DrawRectangleRec(tiles[START_X][START_Y].rectangle, YELLOW);
            DrawText(time_buffer, button_dijkstra.x, button_dijkstra.y - 40, 24, BLACK);
            DrawRectangleRec(button_reset, RED);
            DrawRectangleRec(button_dijkstra, YELLOW);
            DrawFPS(SCREENWIDTH -80, SCREENHEIGHT - 80);
        }
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context
    de_init_tiles();
    
    return 0;
}

void init_queue(queue_t *q)
{
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

bool is_queue_empty(queue_t *q)
{
    if(q->rear == NULL)
    {
        queue_is_empty = true;
        return true;
    }
    else
    {
        queue_is_empty = false;
        return false;
    }
}

void enqueue(queue_t *q, tile_position_t pos)
{
    node_t *tmp;
    tmp = malloc(sizeof(node_t));
    tmp->tile_position = pos;
    tmp->next = NULL;
    if(is_queue_empty(q) == false)
    {
        q->rear->next = tmp;
        q->rear = tmp;
    }
    else
    {
        q->front = q->rear = tmp;
    }
    q->count++;
}

tile_position_t dequeue(queue_t *q)
{
    node_t *tmp;
    if(q->count != 0)
    {
        tile_position_t n = q->front->tile_position;
        tmp = q->front;
        q->front = q->front->next;
        q->count--;
        free(tmp);
        return n;
    }
    else
    {
        return (tile_position_t){0,0};
    }
}

void draw_tiles(void)
{
    for (size_t row = 0; row < ROWS; row++)
    {
        for (size_t column = 0; column < COLUMS; column++)
        {
            DrawRectangleRec(tiles[row][column].rectangle, tiles[row][column].color);
        }
    }
}

void init_tiles(void)
{
    for (size_t row = 0; row < ROWS; row++)
    {
        for (size_t column = 0; column < COLUMS; column++)
        {
            tiles[row][column].tile_position.row = row;
            tiles[row][column].tile_position.column = column;
            tiles[row][column].rectangle.height = NODEHEIGHT;
            tiles[row][column].rectangle.width = NODEWIDTH;
            tiles[row][column].rectangle.x = column * NODEWIDTH;
            tiles[row][column].rectangle.y = row * NODEHEIGHT;
            // "chessboard"-coloring
            if((row+column) % 2)
            {
                tiles[row][column].color = BLUE;
            }
            else
            {
                tiles[row][column].color = SKYBLUE;
            }

            tiles[row][column].weight = GetRandomValue(1, WEIGHTS);
            tiles[row][column].finalized = false;

            // allocate space for 4 neighbours
            tiles[row][column].neighbours = malloc(sizeof(tile_position_t) * 4); 
            // north tile
            tiles[row][column].neighbours[0].row = row - 1;
            tiles[row][column].neighbours[0].column = column;
            // east tile
            tiles[row][column].neighbours[1].row = row;
            tiles[row][column].neighbours[1].column = column + 1;      
            // south tile
            tiles[row][column].neighbours[2].row = row + 1;
            tiles[row][column].neighbours[2].column = column;
            // west tile
            tiles[row][column].neighbours[3].row = row;
            tiles[row][column].neighbours[3].column = column - 1;
        }
    }
}

// rule of thumb: every malloc needs a free. Thus the de_init
void de_init_tiles(void)
{
    for (size_t row = 0; row < ROWS; row++)
    {
        for (size_t column = 0; column < COLUMS; column++)
        {
            free(tiles[row][column].neighbours);
        }
    }
}

void set_tile_path(void)
{
    for (size_t i = 0; i < PATHDRAWSPEED; i++)
    {
        if(!queue_is_empty)
        {
            draw_position = dequeue(algorythm_steps);
            tiles[draw_position.row][draw_position.column].color = GREEN;
        }   
    }
}


void dijkstra_update_distance(tile_position_t current_u)
{
    int adjacent_row;
    int adjacent_column;
    int tmp_reach_cost;

    for (size_t i = 0; i < 4; i++)
    {
        adjacent_row = tiles[current_u.row][current_u.column].neighbours[i].row;
        adjacent_column = tiles[current_u.row][current_u.column].neighbours[i].column;
        if ((tiles[adjacent_row][adjacent_column].finalized == false) && (adjacent_row != -1) && (adjacent_column != -1))
        {
            tmp_reach_cost = tiles[current_u.row][current_u.column].reach_cost + tiles[adjacent_row][adjacent_column].weight;
            if(tmp_reach_cost < tiles[adjacent_row][adjacent_column].reach_cost)
            {
                tiles[adjacent_row][adjacent_column].reach_cost = tmp_reach_cost;
            }
        }
    }
}

// tried to follow this explanation : https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
void dijkstra(tile_position_t startnode)
{
    // init algorythm informations. (all to costs to inifity, not min distance not yet found)
    for (size_t row = 0; row < ROWS; row++)
    {
        for (size_t column = 0; column < COLUMS; column++)
        {
            tiles[row][column].finalized = false;

            if((row == startnode.row) && (column == startnode.column))
            {
                tiles[row][column].reach_cost = 0;
            }
            else
            {
                tiles[row][column].reach_cost = INT_MAX;
            }
        }
    }
    queue_is_empty = false;
    bool finished = false;
    int min_dist = INT_MAX;
    int tmp_min_dist = INT_MAX;
    tile_position_t current_u;
    while (!finished)
    {
        min_dist = INT_MAX;
        for (size_t row = 0; row < ROWS; row++)
        {
            for (size_t column = 0; column < COLUMS; column++)
            {
                if(tiles[row][column].finalized == false)
                {
                    tmp_min_dist = tiles[row][column].reach_cost;
                    if(tmp_min_dist < min_dist)
                    {
                        min_dist = tmp_min_dist;
                        current_u = (tile_position_t){row, column};
                    }
                }
            }
        }
        tiles[current_u.row][current_u.column].finalized = true;
        dijkstra_update_distance(current_u);

        finished = true;
        for (size_t row = 0; row < ROWS; row++)
        {
            for (size_t column = 0; column < COLUMS; column++)
            {
                if(tiles[row][column].finalized == false)
                {
                    finished = false;
                }
            }
        }
        enqueue(algorythm_steps, current_u);
    }
}