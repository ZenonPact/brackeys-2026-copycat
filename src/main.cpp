#include "raylib.h"
#include "raymath.h"

int main()
{
    const int SCREEN_WIDTH = 960;
    const int SCREEN_HEIGHT = 540;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CopyCat");

    SetTargetFPS(60);
    Vector2 position = Vector2{480.0f,270.0f};
    Vector2 direction = Vector2{0.0f,0.0f};
    float speed = 200.0f;


    while(!WindowShouldClose())
    {
        direction = Vector2{0.0,0.0};
        
        if(IsKeyDown(KEY_UP))
        {
            direction.y += -1;
        }
        if(IsKeyDown(KEY_DOWN))
        {
            direction.y += 1;
        }
        if(IsKeyDown(KEY_RIGHT))
        {
            direction.x += 1;
        }
        if(IsKeyDown(KEY_LEFT))
        {
            direction.x += -1;
        }
        direction = Vector2Normalize(direction);
        position = Vector2Add(position, Vector2Scale(direction, speed*GetFrameTime()));

        BeginDrawing();
            ClearBackground(LIGHTGRAY);
            DrawCircleV(position, 20.0f, RED);
            DrawText("Trust No One", 0, 0, 48, BLACK);
        EndDrawing();
    }

    CloseWindow();
                //position = Vector2Add(position, Vector2Scale(Vector2{0.0f,-1.0f}, speed*GetFrameTime()));
    return 0;
}
