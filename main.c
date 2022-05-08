#include "include\raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define SCREENWIDTH 800
#define SCREENHEIGHT 600
#define ROWS 20
#define COLUMS 20
#define NODEWIDTH 20
#define NODEHEIGHT 20

typedef struct
{
    int rowPositon;
    int columnPosition;
    Rectangle rectangle;
    Color color;
}node_t;

node_t nodes[ROWS][COLUMS] = {0};

void draw_nodes(void)
{
    for (size_t i = 0; i < ROWS; i++)
    {
        for (size_t j = 0; j < COLUMS; j++)
        {
            DrawRectangleRec(nodes[i][j].rectangle, nodes[i][j].color);
        }
    }

}

void init_nodes(void)
{
    for (size_t i = 0; i < ROWS; i++)
    {
        for (size_t j = 0; j < COLUMS; j++)
        {
            nodes[i][j].rowPositon = i;
            nodes[i][j].columnPosition = j;
            nodes[i][j].rectangle.height = NODEHEIGHT;
            nodes[i][j].rectangle.width = NODEWIDTH;
            nodes[i][j].rectangle.x = j * NODEWIDTH;
            nodes[i][j].rectangle.y = i * NODEHEIGHT;
            // grid color
            if((i+j) % 2)
            {
                nodes[i][j].color = BLUE;
            }
            else
            {
                nodes[i][j].color = SKYBLUE;
            }
        }
    }
}

int main(void)
{
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Test");
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    
    init_nodes();

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        BeginDrawing();
        {
            
            ClearBackground(WHITE);

            draw_nodes();

            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 pos = GetMousePosition();
                int rowIndex = pos.y/NODEHEIGHT;
                int colIndex = pos.x/NODEWIDTH;
                if((rowIndex < ROWS) && (colIndex < COLUMS))
                {
                    nodes[rowIndex][colIndex].color = RED;
                }
            }
            DrawFPS(0,0);
        
        }
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context

    return 0;
}