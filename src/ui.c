#include "ui.h"
#include "raylib.h"
#include <string.h>

static char customer[CUSTOMER_CAP];
static char subject[SUBJECT_CAP];
static char description[DESCRIPTION_CAP];
static int active_field;
static int first_row;
static int detail_scroll;
static int detail_id;
static const Rectangle detail_area = {448, 414, 480, 316};

static const Ticket *selected_ticket(const AppState *app)
{
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id == app->selected_id) return &app->tickets[i];
    return NULL;
}

/* Wrap at spaces when possible; split long words so every byte is visible.
   The current form accepts ASCII. The same layout measures and draws. */
static int detail_line(const char *text, int y, bool draw)
{
    while (*text) {
        char line[DESCRIPTION_CAP + 80];
        size_t n = 0, space = 0;
        while (text[n] && text[n] != '\n' && n < sizeof line - 1) {
            line[n] = text[n];
            line[n + 1] = '\0';
            if (MeasureText(line, 18) > 448 && n > 0) break;
            if (text[n] == ' ') space = n;
            ++n;
        }
        size_t consumed = n;
        if (text[n] && text[n] != '\n' && space > 0) {
            n = space;
            consumed = space + 1;
        }
        line[n] = '\0';
        if (draw) DrawText(line, 460, y, 18, DARKGRAY);
        y += 24;
        text += consumed;
        if (*text == '\n') ++text;
    }
    return y + 8;
}

static int detail_content(const Ticket *ticket, int y, bool draw)
{
    y = detail_line(TextFormat("ID: #%d", ticket->id), y, draw);
    y = detail_line(TextFormat("Customer: %s", ticket->customer), y, draw);
    y = detail_line(TextFormat("Subject: %s", ticket->subject), y, draw);
    y = detail_line(TextFormat("Status: %s", ticket_status_name(ticket->status)), y, draw);
    y = detail_line(TextFormat("Priority: %s", ticket_priority_name(ticket->priority)), y, draw);
    y = detail_line(TextFormat("Assigned to: %s", ticket->assignee[0] ? ticket->assignee : "Unassigned"), y, draw);
    return detail_line(TextFormat("Description: %s", ticket->description), y, draw);
}
static const Rectangle fields[] = {{32, 133, 380, 38}, {32, 207, 380, 38}, {32, 281, 380, 38}};
static const Rectangle create_button = {32, 342, 380, 42};

static bool clicked(Rectangle box)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), box);
}

static void box(Rectangle area, const char *label, bool active)
{
    DrawRectangleRec(area, active ? SKYBLUE : LIGHTGRAY);
    DrawRectangleLinesEx(area, 1, active ? BLUE : GRAY);
    DrawText(label, (int)area.x + 10, (int)area.y + 11, 18, DARKBLUE);
}

static void clipped_text(const char *text, int x, int y, int width)
{
    BeginScissorMode(x, y, width, 24);
    DrawText(text, x, y, 18, DARKGRAY);
    EndScissorMode();
}

void ui_update(AppState *app)
{
    for (int i = 0; i < 3; ++i) if (clicked(fields[i])) active_field = i;
    if (IsKeyPressed(KEY_TAB)) active_field = (active_field + 1) % 3;
    char *buffers[] = {customer, subject, description};
    const size_t caps[] = {sizeof customer, sizeof subject, sizeof description};
    char *text = buffers[active_field];
    size_t len = strlen(text);
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (len) text[--len] = '\0';
    }
    /* The bundled default font and this initial form use printable ASCII. */
    for (int ch = GetCharPressed(); ch; ch = GetCharPressed()) {
        if (ch >= 32 && ch <= 126 && len + 1 < caps[active_field]) {
            text[len++] = (char)ch;
            text[len] = '\0';
        }
    }
    if (clicked(create_button) || IsKeyPressed(KEY_ENTER)) {
        if (ticket_create(app, customer, subject, description)) {
            customer[0] = subject[0] = description[0] = '\0';
            active_field = 0;
            first_row = app->count > 6 ? (int)app->count - 6 : 0;
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){448, 110, 480, 290}))
        first_row -= (int)GetMouseWheelMove();
    int max_first = app->count > 6 ? (int)app->count - 6 : 0;
    if (first_row < 0) first_row = 0;
    if (first_row > max_first) first_row = max_first;
    for (int row = 0; row < 6 && (size_t)(first_row + row) < app->count; ++row) {
        if (clicked((Rectangle){448, (float)(122 + row * 43), 480, 39}))
            app->selected_id = app->tickets[first_row + row].id;
    }
    if (detail_id != app->selected_id) {
        detail_id = app->selected_id;
        detail_scroll = 0;
    }
    const Ticket *selected = selected_ticket(app);
    int max_scroll = selected ? detail_content(selected, 0, false) - ((int)detail_area.height - 24) : 0;
    if (max_scroll < 0) max_scroll = 0;
    if (CheckCollisionPointRec(GetMousePosition(), detail_area))
        detail_scroll -= (int)(GetMouseWheelMove() * 32);
    if (detail_scroll < 0) detail_scroll = 0;
    if (detail_scroll > max_scroll) detail_scroll = max_scroll;
    for (int status = 0; status < 3; ++status) {
        if (selected && clicked((Rectangle){448 + status * 164.0f, 750, 152, 38}))
            ticket_set_status(app, app->selected_id, (TicketStatus)status);
    }
}

void ui_draw(const AppState *app)
{
    DrawText("Customer Support Platform", 32, 24, 30, DARKBLUE);
    DrawText("Create tickets and update their status", 32, 65, 20, DARKGRAY);
    const char *labels[] = {"Customer", "Subject", "Description"};
    const char *values[] = {customer, subject, description};
    const int limits[] = {CUSTOMER_CAP - 1, SUBJECT_CAP - 1, DESCRIPTION_CAP - 1};
    for (int i = 0; i < 3; ++i) {
        DrawText(TextFormat("%s  (%d/%d)", labels[i], (int)strlen(values[i]), limits[i]),
                 32, (int)fields[i].y - 24, 18, DARKGRAY);
        DrawRectangleRec(fields[i], WHITE);
        DrawRectangleLinesEx(fields[i], 2, active_field == i ? BLUE : LIGHTGRAY);
        const char *visible = values[i];
        while (*visible && MeasureText(visible, 18) > 358) ++visible;
        clipped_text(visible, 42, (int)fields[i].y + 10, 358);
    }
    box(create_button, "Create ticket  [Enter]", false);
    DrawText("Tab: next field. English text input.", 32, 402, 17, GRAY);
    DrawText("Scroll over the list to view more tickets.", 448, 97, 16, GRAY);
    if (!app->count) DrawText("No tickets yet. Create your first ticket.", 448, 140, 18, GRAY);
    for (int row = 0; row < 6 && (size_t)(first_row + row) < app->count; ++row) {
        const Ticket *ticket = &app->tickets[first_row + row];
        Rectangle area = {448, (float)(122 + row * 43), 480, 39};
        box(area, "", app->selected_id == ticket->id);
        clipped_text(TextFormat("#%d  %s", ticket->id, ticket->subject), 458, (int)area.y + 10, 300);
        DrawText(ticket_status_name(ticket->status), 770, (int)area.y + 11, 16, DARKBLUE);
    }
    const Ticket *selected = selected_ticket(app);
    DrawText("Selected ticket", 448, 389, 20, DARKBLUE);
    DrawRectangleRec(detail_area, WHITE);
    DrawRectangleLinesEx(detail_area, 1, LIGHTGRAY);
    BeginScissorMode(449, 415, 478, 314);
    if (selected)
        detail_content(selected, 426 - detail_scroll, true);
    else
        DrawText("Select a ticket to view its details.", 460, 430, 18, GRAY);
    EndScissorMode();
    if (selected && detail_content(selected, 0, false) > (int)detail_area.height - 24)
        DrawText("Scroll inside the card for full details", 448, 732, 14, GRAY);
    for (int status = 0; status < 3; ++status)
        box((Rectangle){448 + status * 164.0f, 750, 152, 38}, ticket_status_name((TicketStatus)status), selected && selected->status == (TicketStatus)status);
    DrawText(app->error, 32, 512, 18, MAROON);
    DrawText(TextFormat("Tickets: %d / %d | Session only - saving is not implemented", (int)app->count, MAX_TICKETS),
             32, 817, 18, DARKGRAY);
}
