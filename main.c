#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "include/raylib.h"

#define SCREENWIDTH 1000
#define SCREENHEIGHT 800
#define ROWS 40
#define COLUMS 40
#define NODEWIDTH 15
#define NODEHEIGHT 15
#define TARGET_FPS 60
#define START_X 10
#define START_Y 10
#define END_X 30
#define END_Y 30
#define PATHDRAWSPEED 2       // TARGET_FPS * PATHDRAWSPEED = drawn tiles (that are in a queue) per seconds
#define WEIGHTS 1

typedef struct int_Vector_2D{
    int row;
    int column;
} int_Vector_2D;

typedef struct tile_t{
    int_Vector_2D tile_position;
    Rectangle rectangle;
    Color color;
    int weight;
    int reach_cost;
    bool finalized;
    int_Vector_2D *neighbours;    // not a valid neighbour if row or column is -1
    int_Vector_2D predesessor;
} tile_t;

typedef struct node_t{
    int_Vector_2D tile_position;
    struct node_t* next;
} node_t;

typedef struct queue_t{
    int count;
    node_t *front;
    node_t *rear;
} queue_t;

// variables
int_Vector_2D start_node;
int_Vector_2D end_node;
static queue_t *search_path;
static queue_t *shortest_path;
static int_Vector_2D draw_position;
static tile_t tiles[ROWS][COLUMS];
double time = 0;
char time_buffer[50];
double timeout = 0;
Rectangle button_reset;
Rectangle button_dijkstra;

// UI
void left_mouse_is_down(void);
void init_buttons(void);

// queue functions
bool is_queue_empty(queue_t *q);
void init_queue(queue_t *q);
void enqueue(queue_t *q, int_Vector_2D pos);
int_Vector_2D dequeue(queue_t *q);

// tile functions
void draw_tiles(void);
void init_tiles(void);
void de_init_tiles(void);
void set_search_path_tile(void);
void set_shortest_path_tile(void);

// algorythm functions
void dijkstra(int_Vector_2D startnode, int_Vector_2D endnode);
bool dijkstra_all_tiles_finalized();
void dijkstra_update_distance(int_Vector_2D current_u);

int main(void){
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Test");
    SetTargetFPS(TARGET_FPS);                   // Set our game to run at 60 frames-per-second

    init_tiles();
    init_buttons();
    search_path = malloc(sizeof(queue_t));
    shortest_path = malloc(sizeof(queue_t));
    init_queue(search_path);
    init_queue(shortest_path);
    start_node = (int_Vector_2D){START_X, START_Y};
    end_node = (int_Vector_2D){END_X, END_Y};

    // Main game loop
    while (!WindowShouldClose()){  
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            left_mouse_is_down();
        }

        if(!is_queue_empty(search_path)){
            set_search_path_tile();
        }

        if((search_path->count == 0) && (shortest_path->count > 0)){
            set_shortest_path_tile();
        }

        // Rendering
        BeginDrawing();
        {
            ClearBackground(WHITE);
            draw_tiles();
            DrawRectangleRec(tiles[START_X][START_Y].rectangle, YELLOW);
            DrawRectangleRec(tiles[END_X][END_Y].rectangle, BLACK);
            DrawText(time_buffer, button_dijkstra.x, button_dijkstra.y - 40, 24, BLACK);
            DrawRectangleRec(button_reset, RED);
            DrawRectangleRec(button_dijkstra, YELLOW);
            DrawFPS(SCREENWIDTH -80, SCREENHEIGHT - 80);
        }
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();
    de_init_tiles();
    free(search_path);
    return 0;
}

void init_buttons(void){
    // RESET
    button_reset.height = 60;
    button_reset.width = 120;
    button_reset.x = SCREENWIDTH - 300;
    button_reset.y = SCREENHEIGHT - 80;

    // START DIJKSTRA
    button_dijkstra.height = 60;
    button_dijkstra.width = 120;
    button_dijkstra.x = SCREENWIDTH - 600;
    button_dijkstra.y = SCREENHEIGHT - 80;
}

void left_mouse_is_down(void){
    Vector2 pos = GetMousePosition();
    uint8_t rowIndex = (uint8_t) (pos.y/NODEHEIGHT);
    uint8_t colIndex = (uint8_t) (pos.x/NODEWIDTH);
    if((rowIndex < ROWS) && (colIndex < COLUMS)){
        tiles[rowIndex][colIndex].color = RED;
        tiles[rowIndex][colIndex].weight = 50000;
        if(rowIndex > 0 && colIndex >0 && colIndex < ROWS && colIndex < COLUMS){
            tiles[rowIndex+1][colIndex].color = RED;
            tiles[rowIndex+1][colIndex].weight = 50000;               
            tiles[rowIndex][colIndex+1].color = RED;
            tiles[rowIndex][colIndex+1].weight = 50000;
        }
    }

    if(GetTime()-timeout >= 0.5f){
        timeout = GetTime();
        if(CheckCollisionPointRec(pos, button_dijkstra)){ 
            time = GetTime();
            dijkstra(start_node ,end_node);
            time = GetTime() - time;
            sprintf(time_buffer, "%.10f [ms]", time * 1000);
        }
        if(CheckCollisionPointRec(pos, button_reset)){
            init_queue(search_path);
            init_queue(shortest_path);
            de_init_tiles();
            init_tiles();
        }
    }
}


void init_queue(queue_t *q){
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

bool is_queue_empty(queue_t *q){
    if(q->count == 0){
        return true;
    }
    else{
        return false;
    }
}

void enqueue(queue_t *q, int_Vector_2D pos){
    node_t *tmp;
    tmp = malloc(sizeof(node_t));
    tmp->tile_position = pos;
    tmp->next = NULL;
    if(is_queue_empty(q) == false){
        q->rear->next = tmp;
        q->rear = tmp;
    }
    else{
        q->front = q->rear = tmp;
    }
    q->count++;
}

int_Vector_2D dequeue(queue_t *q){
    node_t *tmp;
    if(q->count != 0){
        int_Vector_2D n = q->front->tile_position;
        tmp = q->front;
        q->front = q->front->next;
        q->count--;
        free(tmp);
        return n;
    }
    else{
        return (int_Vector_2D){0,0};
    }
}

void draw_tiles(void){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t column = 0; column < COLUMS; column++){
            DrawRectangleRec(tiles[row][column].rectangle, tiles[row][column].color);
        }
    }
}

void init_tiles(void){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t column = 0; column < COLUMS; column++){
            tiles[row][column].tile_position.row = row;
            tiles[row][column].tile_position.column = column;
            tiles[row][column].rectangle.height = NODEHEIGHT;
            tiles[row][column].rectangle.width = NODEWIDTH;
            tiles[row][column].rectangle.x = column * NODEWIDTH;
            tiles[row][column].rectangle.y = row * NODEHEIGHT;
            // "chessboard"-coloring
            if((row+column) % 2){
                tiles[row][column].color = BLUE;
            }
            else if ((row == end_node.row) && (column == end_node.column)){
                tiles[row][column].color = GOLD;
            }
            else{
                tiles[row][column].color = SKYBLUE;
            }

            tiles[row][column].weight = GetRandomValue(1, WEIGHTS);
            tiles[row][column].finalized = false;

            // allocate space for 4 neighbours
            tiles[row][column].neighbours = malloc(sizeof(int_Vector_2D) * 4); 
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
void de_init_tiles(void){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t column = 0; column < COLUMS; column++){
            free(tiles[row][column].neighbours);
        }
    }
}

void set_search_path_tile(void){
    for (size_t i = 0; i < PATHDRAWSPEED; i++){
        draw_position = dequeue(search_path);
        tiles[draw_position.row][draw_position.column].color = GREEN; 
    }
}

void set_shortest_path_tile(void){
    for (size_t i = 0; i < PATHDRAWSPEED; i++){
        draw_position = dequeue(shortest_path);
        tiles[draw_position.row][draw_position.column].color = BLACK; 
    }
}

// orientated on following explanation : https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
void dijkstra(int_Vector_2D startnode, int_Vector_2D endnode){

    // STEP: Init all distances to INIFINTE. Assign distatnce for source vertex to 0
    for (size_t row = 0; row < ROWS; row++){
        for (size_t column = 0; column < COLUMS; column++){
            tiles[row][column].finalized = false;   // set to => "shortest path not found yet"
            if((row == startnode.row) && (column == startnode.column)){
                tiles[row][column].reach_cost = 0;
            }
            else{
                tiles[row][column].reach_cost = INT_MAX;
            }
        }
    }
    bool finished = false;
    int min_dist = INT_MAX;
    int_Vector_2D current_u;
    int_Vector_2D shortest;

    // while still tiles in array, that are not finalized
    while (!finished){
        min_dist = INT_MAX;

        // STEAP: search tile with minimum distance, which is not finalized
        for (size_t row = 0; row < ROWS; row++){
            for (size_t column = 0; column < COLUMS; column++){
                if(!tiles[row][column].finalized){
                    if(tiles[row][column].reach_cost < min_dist){
                        min_dist = tiles[row][column].reach_cost;
                        current_u = (int_Vector_2D){row, column};
                    }
                }
            }
        }

        // current_u has minumum distance, so it is finalized
        tiles[current_u.row][current_u.column].finalized = true;
        // calculate neighbour distances from current_u
        dijkstra_update_distance(current_u);
        // add to queue for new finalized tile
        enqueue(search_path, current_u);
        // STEP: All tiles finalized?
        
        finished = dijkstra_all_tiles_finalized();
        
        if((current_u.row == endnode.row) && (current_u.column == endnode.column)){
            
            while ((current_u.row != start_node.row) || (current_u.column != start_node.column))
            {
                shortest = tiles[current_u.row][current_u.column].predesessor;
                enqueue(shortest_path, current_u);
                current_u = shortest;
            }
            finished = true;
        }
    }
}

bool dijkstra_all_tiles_finalized(void){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t column = 0; column < COLUMS; column++){
            if(tiles[row][column].finalized == false){
                return false;
            }
        }
    }
    return true;
}

void dijkstra_update_distance(int_Vector_2D current_u){
    int adjacent_row;
    int adjacent_column;
    int tmp_reach_cost;

    for (size_t i = 0; i < 4; i++){
        adjacent_row = tiles[current_u.row][current_u.column].neighbours[i].row;
        adjacent_column = tiles[current_u.row][current_u.column].neighbours[i].column;

        if ((tiles[adjacent_row][adjacent_column].finalized == false) && (adjacent_row != -1) && (adjacent_column != -1)){
            tmp_reach_cost = tiles[current_u.row][current_u.column].reach_cost + tiles[adjacent_row][adjacent_column].weight;
            if(tmp_reach_cost < tiles[adjacent_row][adjacent_column].reach_cost){
                tiles[adjacent_row][adjacent_column].reach_cost = tmp_reach_cost;
                tiles[adjacent_row][adjacent_column].predesessor = current_u;
            }
        }
    }
}