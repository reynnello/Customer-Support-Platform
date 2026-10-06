#include "app.h"
#include "ui.h"
#include "raylib.h"

int main(void)
{
    AppState app;
    app_init(&app);
    InitWindow(960, 600, "Customer Support Platform");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        ui_update(&app);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        ui_draw(&app);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
