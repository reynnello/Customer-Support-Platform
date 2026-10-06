#include "ui.h"
#include "raylib.h"

void ui_update(AppState *app)
{
    (void)app; /* Anton: add input handling here. */
}

void ui_draw(const AppState *app)
{
    DrawText("Customer Support Platform", 32, 32, 30, DARKBLUE);
    DrawText("Shared C + Raylib foundation", 32, 80, 20, DARKGRAY);
    DrawText(TextFormat("Tickets: %d / %d", (int)app->count, MAX_TICKETS),
             32, 132, 22, DARKGRAY);
    DrawText("No tickets yet. Features will be added in personal branches.",
             32, 180, 18, GRAY);
    DrawText("ESC to close", 32, 530, 18, GRAY);
}
