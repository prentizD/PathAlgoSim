#include <iostream>
#include <string>
#include "raylib.h"

const int screenWidth = 800;
const int screenHeight = 600;
static const int rows = 20;
static const int colums = 20;
static const int nodeHeight = screenHeight/rows; 
static const int nodeWidth = screenWidth/colums;
static Color nodeColor = {BLUE};

struct Node
{
    Rectangle rec;
    Color col;
};


int main(void)
{
    InitWindow(screenWidth, screenHeight, "Test");
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    Node nodes[rows][colums];

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < colums; j++)
        {
            nodes[i][j].rec.x = j*(screenWidth/colums);
            nodes[i][j].rec.y = i*(screenHeight/rows);
            nodes[i][j].rec.width = nodeWidth;
            nodes[i][j].rec.height = nodeHeight;
            if((i+j) % 2)
            {
                nodes[i][j].col = Color{BLUE};
            }
            else
            {
                nodes[i][j].col = Color{SKYBLUE};
            }
        }
    }

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        BeginDrawing();
            ClearBackground(BLACK);
            //DrawText(std::to_string(rows).c_str(), 300, 20, 12, Color{YELLOW});

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < colums; j++)
                {
                    DrawRectangleRec(nodes[i][j].rec, nodes[i][j].col);
                }
            }

            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 pos = GetMousePosition();
                int rowIndex = pos.y/nodeHeight;
                int colIndex = pos.x/nodeWidth;
                if((rowIndex < rows) && (colIndex < colums))
                {
                    nodes[rowIndex][colIndex].col = {RED};
                }
            }
            
            DrawFPS(0,0);
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context

    return 0;
}
