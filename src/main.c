#include "raylib.h"
#include <stddef.h>

typedef enum { OPEN, IN_PROGRESS, RESOLVED } TicketStatus;
typedef struct {
    int id;
    const char *customer;
    const char *subject;
    const char *description;
    TicketStatus status;
} Ticket;

static const char *status_names[] = { "Open", "In progress", "Resolved" };
static const Color ink = { 28, 40, 57, 255 };
static const Color muted = { 92, 107, 126, 255 };
static const Color accent = { 37, 99, 190, 255 };

static bool Button(Rectangle bounds, const char *label, bool active)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), bounds);
    DrawRectangleRec(bounds, active ? accent : (hover ? (Color){218, 230, 247, 255} : (Color){234, 239, 246, 255}));
    DrawText(label, (int)bounds.x + 14, (int)bounds.y + 14, 18, active ? WHITE : ink);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int main(void)
{
    Ticket tickets[] = {
        {1001, "Alex Morgan", "Cannot sign in", "Password reset email has not arrived.\nPlease check the account email settings.", OPEN},
        {1002, "Sam Taylor", "Invoice question", "Customer needs a copy of the latest invoice.\nConfirm the billing period before replying.", IN_PROGRESS},
        {1003, "Jamie Lee", "Update contact details", "Customer contact details have been updated.\nThe customer confirmed the change.", RESOLVED}
    };
    const int count = (int)(sizeof(tickets) / sizeof(tickets[0]));
    int selected = 0;
    InitWindow(1100, 700, "Customer Support Platform | Session 1");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        int totals[3] = {0};
        for (int i = 0; i < count; ++i) totals[tickets[i].status]++;
        BeginDrawing();
        ClearBackground((Color){245, 247, 251, 255});
        DrawRectangle(0, 0, 1100, 108, ink);
        DrawText("Customer Support", 32, 26, 32, WHITE);
        DrawText("Team workspace / Prototype 01", 33, 70, 18, (Color){188, 203, 224, 255});
        for (int i = 0; i < 3; ++i) {
            int x = 32 + i * 354;
            DrawRectangle(x, 132, 330, 84, WHITE);
            DrawText(status_names[i], x + 18, 148, 18, muted);
            DrawText(TextFormat("%d", totals[i]), x + 18, 177, 26, ink);
        }
        DrawText("Tickets", 32, 244, 24, ink);
        DrawText("Select a ticket to view its details", 32, 279, 17, muted);
        for (int i = 0; i < count; ++i) {
            Rectangle row = {32, (float)(315 + i * 94), 360, 80};
            bool hover = CheckCollisionPointRec(GetMousePosition(), row);
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selected = i;
            DrawRectangleRec(row, selected == i ? (Color){224, 235, 252, 255} : WHITE);
            if (selected == i) DrawRectangle((int)row.x, (int)row.y, 4, 80, accent);
            DrawText(TextFormat("#%d  %s", tickets[i].id, tickets[i].customer), 48, (int)row.y + 12, 17, muted);
            DrawText(tickets[i].subject, 48, (int)row.y + 42, 20, ink);
        }
        Ticket *ticket = &tickets[selected];
        DrawRectangle(416, 244, 652, 372, WHITE);
        DrawText(TextFormat("TICKET #%d", ticket->id), 440, 269, 18, muted);
        DrawText(ticket->subject, 440, 307, 28, ink);
        DrawText(TextFormat("Customer: %s", ticket->customer), 440, 353, 19, muted);
        DrawText(ticket->description, 440, 402, 18, ink);
        DrawText("Status", 440, 493, 18, muted);
        for (int i = 0; i < 3; ++i) {
            if (Button((Rectangle){(float)(440 + i * 196), 528, 180, 48}, status_names[i], ticket->status == (TicketStatus)i))
                ticket->status = (TicketStatus)i;
        }
        DrawText("Demo data only. Changes reset when the app closes.", 32, 649, 18, muted);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
