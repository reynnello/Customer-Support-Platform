#include "app.h"
#include "ui.h"
#include "raylib.h"

int main(void)
{
    AppState app;
    app_init(&app);
    InitWindow(960, 950, "Customer Support Platform");
    SetTargetFPS(60);
    bool allow_autosave = true;
    if (FileExists(DEFAULT_DATA_FILE))
        allow_autosave = storage_load(&app, DEFAULT_DATA_FILE);
    while (!WindowShouldClose()) {
        ui_update(&app);
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
            storage_save(&app, DEFAULT_DATA_FILE);
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        ui_draw(&app);
        EndDrawing();
    }
    if (app.dirty && allow_autosave && !storage_save(&app, DEFAULT_DATA_FILE))
        TraceLog(LOG_ERROR, "%s", app.error);
    CloseWindow();
    return 0;
}
