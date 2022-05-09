#include "include\raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>

#define SCREENWIDTH 800
#define SCREENHEIGHT 600
#define ROWS 50
#define COLUMS 80
#define NODEWIDTH 10
#define NODEHEIGHT 10

typedef struct
{
    int row;
    int column;
} tile_position_t;

typedef struct node{
    tile_position_t tile_position;
    struct node* next;
} node_t;

typedef struct
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

node_t *algorythm_path;

int isEmpty() {
    if (algorythm_path == NULL) 
        return 1;
    else
        return 0;
}

void push_node(tile_position_t num)
{
    struct node *temp;
    temp =(struct node *)malloc(1*sizeof(struct node));
    temp->tile_position = num;
     
    if (algorythm_path == NULL) 
    {
        algorythm_path = temp;
        algorythm_path->next = NULL;
    }
    else
    {
        temp->next = algorythm_path;
        algorythm_path = temp;
    }
}

tile_position_t pop_node() 
{
    struct node *temp;
    if (isEmpty(algorythm_path)) 
    {
        return (tile_position_t){0};
    } 
    else 
    {
        temp = algorythm_path;
        algorythm_path = algorythm_path->next;
        free(temp); 
        return algorythm_path->tile_position;
    }
}


tile_t tiles[ROWS][COLUMS] = {0}; // init all variables to 0

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

            tiles[row][column].weight = GetRandomValue(0,1);
            if(row %2 == 0)
            {
                tiles[row][column].weight = GetRandomValue(0,2000);
            }
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

tile_position_t draw_position;

#define PATHSPEED 10
void set_tile_path(void)
{
    for (size_t i = 0; i < PATHSPEED; i++)
    {
        if((algorythm_path->next != NULL))
        {
            draw_position = pop_node();
            tiles[draw_position.row][draw_position.column].color = BLACK;
        }   
    }
    

}


void dijkstra(tile_position_t startnode);

int main(void)
{
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Test");
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    init_tiles();
    tile_position_t start_node = {ROWS/2,COLUMS/2};
    dijkstra(start_node);
    
    float elapsed_frametime = 0.0f;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {  

        // loop logic
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 pos = GetMousePosition();
            int rowIndex = pos.y/NODEHEIGHT;
            int colIndex = pos.x/NODEWIDTH;
            if((rowIndex < ROWS) && (colIndex < COLUMS))
            {
                tiles[rowIndex][colIndex].color = RED;
            }
            set_tile_path();
        }


        // drawing
        BeginDrawing();
        {
            ClearBackground(WHITE);
            draw_tiles();
            DrawFPS(SCREENWIDTH -80, SCREENHEIGHT - 80);
        }
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context
    de_init_tiles();
    
    return 0;
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
    
    bool finished = false;
    int min_dist = INT_MAX;
    int tmp_min_dist = INT_MAX;
    tile_position_t current_u;
    tile_position_t allInOrder[ROWS*COLUMS];
    int i = 0;
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

        allInOrder[i] = current_u;
        i++;
        
        //printf("Row: %d Col: %d Reach_Cost: %d \n", current_u.row, current_u.column, tiles[current_u.row][current_u.column].reach_cost);
    }

    for (size_t j = ROWS*COLUMS; j > 0; j--)
    {
        push_node(allInOrder[j-1]);
    }
        
}