#include "ui.h"
#include "raylib.h"
#include <string.h>

static char customer[CUSTOMER_CAP];
static char subject[SUBJECT_CAP];
static char description[DESCRIPTION_CAP];
static char search[SUBJECT_CAP];
static char assignee[ASSIGNEE_CAP];
static int active_field;
static int first_row;
static int detail_scroll;
static int detail_id;
static int status_filter = -1;
static int priority_filter = -1;
static int visible_ids[MAX_TICKETS];
static size_t visible_count;
static const Rectangle detail_area = {448, 414, 480, 316};
static const Rectangle search_field = {448, 122, 480, 38};
static const Rectangle status_filter_button = {448, 170, 178, 38};
static const Rectangle priority_filter_button = {638, 170, 178, 38};
static const Rectangle reset_filter_button = {828, 170, 100, 38};
static const Rectangle priority_buttons[] = {
    {32, 469, 120, 38}, {162, 469, 120, 38}, {292, 469, 120, 38}
};
static const Rectangle assignee_field = {32, 548, 246, 38};
static const Rectangle assign_button = {288, 548, 124, 38};
static const Rectangle save_button = {448, 818, 180, 42};

static const Ticket *selected_ticket(const AppState *app)
{
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id == app->selected_id) return &app->tickets[i];
    return NULL;
}

static void refresh_visible(const AppState *app)
{
    visible_count = ticket_query(app, search, status_filter, priority_filter,
                                 visible_ids, MAX_TICKETS);
    int max_first = visible_count > 4 ? (int)visible_count - 4 : 0;
    if (first_row < 0) first_row = 0;
    if (first_row > max_first) first_row = max_first;
}

static const Ticket *ticket_by_id(const AppState *app, int id)
{
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id == id) return &app->tickets[i];
    return NULL;
}

static Color status_color(TicketStatus status)
{
    switch (status) {
        case TICKET_OPEN: return BLUE;
        case TICKET_IN_PROGRESS: return (Color){172, 88, 0, 255};
        case TICKET_RESOLVED: return DARKGREEN;
        default: return DARKGRAY;
    }
}

/* Wrap at spaces when possible; split long words so every byte is visible.
   The current form accepts ASCII. The same layout measures and draws. */
static int detail_line(const char *text, int y, bool draw, Color color)
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
        if (draw) DrawText(line, 460, y, 18, color);
        y += 24;
        text += consumed;
        if (*text == '\n') ++text;
    }
    return y + 8;
}

static int detail_content(const Ticket *ticket, int y, bool draw)
{
    y = detail_line(TextFormat("ID: #%d", ticket->id), y, draw, DARKGRAY);
    y = detail_line(TextFormat("Customer: %s", ticket->customer), y, draw, DARKGRAY);
    y = detail_line(TextFormat("Subject: %s", ticket->subject), y, draw, DARKGRAY);
    y = detail_line(TextFormat("Status: %s", ticket_status_name(ticket->status)), y, draw, status_color(ticket->status));
    y = detail_line(TextFormat("Priority: %s", ticket_priority_name(ticket->priority)), y, draw, DARKGRAY);
    y = detail_line(TextFormat("Assigned to: %s", ticket->assignee[0] ? ticket->assignee : "Unassigned"), y, draw, DARKGRAY);
    return detail_line(TextFormat("Description: %s", ticket->description), y, draw, DARKGRAY);
}
static const Rectangle fields[] = {{32, 133, 380, 38}, {32, 207, 380, 38}, {32, 281, 380, 38}};
static const Rectangle create_button = {32, 342, 234, 42};
static const Rectangle clear_button = {278, 342, 134, 42};

static bool clicked(Rectangle box)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), box);
}

static void box(Rectangle area, const char *label, bool active)
{
    DrawRectangleRec(area, active ? SKYBLUE : LIGHTGRAY);
    DrawRectangleLinesEx(area, 1, active ? BLUE : GRAY);
    int font_size = MeasureText(label, 18) > (int)area.width - 20 ? 16 : 18;
    DrawText(label, (int)area.x + 10, (int)area.y + 11, font_size, DARKBLUE);
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
    if (clicked(search_field)) active_field = 3;
    if (clicked(assignee_field)) active_field = 4;
    if (IsKeyPressed(KEY_TAB)) active_field = (active_field + 1) % 5;
    char *buffers[] = {customer, subject, description, search, assignee};
    const size_t caps[] = {sizeof customer, sizeof subject, sizeof description,
                           sizeof search, sizeof assignee};
    char *text = buffers[active_field];
    size_t len = strlen(text);
    bool search_changed = false;
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (len) {
            text[--len] = '\0';
            search_changed = active_field == 3;
        }
    }
    /* The bundled default font and this initial form use printable ASCII. */
    for (int ch = GetCharPressed(); ch; ch = GetCharPressed()) {
        if (ch >= 32 && ch <= 126 && len + 1 < caps[active_field]) {
            text[len++] = (char)ch;
            text[len] = '\0';
            search_changed = active_field == 3;
        }
    }
    if (search_changed) first_row = 0;
    if (clicked(clear_button)) {
        customer[0] = subject[0] = description[0] = '\0';
        active_field = 0;
        app->error[0] = '\0';
    } else if (clicked(create_button) || (active_field < 3 && IsKeyPressed(KEY_ENTER))) {
        if (ticket_create(app, customer, subject, description)) {
            customer[0] = subject[0] = description[0] = '\0';
            active_field = 0;
            search[0] = '\0';
            status_filter = priority_filter = -1;
            first_row = app->count > 4 ? (int)app->count - 4 : 0;
        }
    }
    if (clicked(status_filter_button)) {
        status_filter = status_filter == TICKET_RESOLVED ? -1 : status_filter + 1;
        first_row = 0;
    }
    if (clicked(priority_filter_button)) {
        priority_filter = priority_filter == PRIORITY_HIGH ? -1 : priority_filter + 1;
        first_row = 0;
    }
    if (clicked(reset_filter_button)) {
        search[0] = '\0';
        status_filter = priority_filter = -1;
        first_row = 0;
    }
    refresh_visible(app);
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){448, 218, 480, 168}))
        first_row -= (int)GetMouseWheelMove();
    int max_first = visible_count > 4 ? (int)visible_count - 4 : 0;
    if (first_row < 0) first_row = 0;
    if (first_row > max_first) first_row = max_first;
    for (int row = 0; row < 4 && (size_t)(first_row + row) < visible_count; ++row) {
        if (clicked((Rectangle){448, (float)(218 + row * 43), 480, 39}))
            app->selected_id = visible_ids[first_row + row];
    }
    if (detail_id != app->selected_id) {
        detail_id = app->selected_id;
        detail_scroll = 0;
        const Ticket *ticket = selected_ticket(app);
        if (ticket) {
            memcpy(assignee, ticket->assignee, sizeof assignee);
        } else {
            assignee[0] = '\0';
        }
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
    for (int priority = 0; priority < 3; ++priority) {
        if (selected && clicked(priority_buttons[priority]))
            ticket_set_priority(app, app->selected_id, (TicketPriority)priority);
    }
    if (selected && (clicked(assign_button) || (active_field == 4 && IsKeyPressed(KEY_ENTER))))
        ticket_set_assignee(app, app->selected_id, assignee);
    if (clicked(save_button)) storage_save(app, DEFAULT_DATA_FILE);
    refresh_visible(app);
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
    box(create_button, "Create ticket", false);
    box(clear_button, "Clear form", false);
    DrawText("Tab: next field. English text input.", 32, 402, 17, GRAY);
    DrawText("Priority", 32, 440, 20, DARKBLUE);
    const Ticket *selected = selected_ticket(app);
    for (int priority = 0; priority < 3; ++priority) {
        Rectangle button = priority_buttons[priority];
        bool active = selected && selected->priority == (TicketPriority)priority;
        DrawRectangleRec(button, active ? Fade(BLUE, 0.18f) : LIGHTGRAY);
        DrawRectangleLinesEx(button, active ? 3 : 1, selected ? BLUE : GRAY);
        DrawText(ticket_priority_name((TicketPriority)priority), (int)button.x + 10,
                 (int)button.y + 11, 18, selected ? DARKBLUE : GRAY);
    }
    DrawText("Assigned to (empty to unassign)", 32, 520, 18, DARKBLUE);
    DrawRectangleRec(assignee_field, WHITE);
    DrawRectangleLinesEx(assignee_field, 2, active_field == 4 ? BLUE : LIGHTGRAY);
    const char *shown_assignee = assignee;
    while (*shown_assignee && MeasureText(shown_assignee, 18) > 222) ++shown_assignee;
    clipped_text(shown_assignee, 42, 558, 222);
    box(assign_button, "Assign", false);
    DrawText("Search by customer or subject", 448, 97, 18, DARKGRAY);
    DrawRectangleRec(search_field, WHITE);
    DrawRectangleLinesEx(search_field, 2, active_field == 3 ? BLUE : LIGHTGRAY);
    const char *shown_search = search;
    while (*shown_search && MeasureText(shown_search, 18) > 456) ++shown_search;
    clipped_text(shown_search, 458, 132, 456);
    box(status_filter_button,
        status_filter == -1 ? "Status: Any" : TextFormat("Status: %s", ticket_status_name((TicketStatus)status_filter)), false);
    box(priority_filter_button,
        priority_filter == -1 ? "Priority: Any" : TextFormat("Priority: %s", ticket_priority_name((TicketPriority)priority_filter)), false);
    box(reset_filter_button, "Reset", false);
    if (!visible_count) {
        DrawText(app->count ? "No tickets match the filters." : "No tickets yet. Create your first ticket.",
                 448, 230, 18, GRAY);
    }
    for (int row = 0; row < 4 && (size_t)(first_row + row) < visible_count; ++row) {
        const Ticket *ticket = ticket_by_id(app, visible_ids[first_row + row]);
        if (!ticket) continue;
        Rectangle area = {448, (float)(218 + row * 43), 480, 39};
        box(area, "", app->selected_id == ticket->id);
        clipped_text(TextFormat("#%d  %s", ticket->id, ticket->subject), 458, (int)area.y + 10, 300);
        DrawText(ticket_status_name(ticket->status), 770, (int)area.y + 11, 16, status_color(ticket->status));
    }
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
    for (int status = 0; status < 3; ++status) {
        Rectangle button = {448 + status * 164.0f, 750, 152, 38};
        bool active = selected && selected->status == (TicketStatus)status;
        Color color = selected ? status_color((TicketStatus)status) : GRAY;
        DrawRectangleRec(button, active ? Fade(color, 0.18f) : LIGHTGRAY);
        DrawRectangleLinesEx(button, active ? 3 : 1, color);
        DrawText(ticket_status_name((TicketStatus)status), (int)button.x + 10,
                 (int)button.y + 11, 18, color);
    }
    box(save_button, "Save (Ctrl+S)", false);
    if (app->dirty) DrawText("Unsaved changes", 642, 830, 18, MAROON);
    else DrawText("No unsaved changes", 642, 830, 18, DARKGREEN);
    /* Derive counts from current tickets each frame, so edits cannot leave
       stale totals and no additional shared state is needed. */
    int counts[4] = {(int)app->count, 0, 0, 0};
    for (size_t i = 0; i < app->count; ++i) {
        switch (app->tickets[i].status) {
            case TICKET_OPEN: ++counts[1]; break;
            case TICKET_IN_PROGRESS: ++counts[2]; break;
            case TICKET_RESOLVED: ++counts[3]; break;
        }
    }
    const char *count_labels[] = {"Total tickets", "Open", "In progress", "Resolved"};
    const Color count_colors[] = {DARKBLUE, status_color(TICKET_OPEN), status_color(TICKET_IN_PROGRESS), status_color(TICKET_RESOLVED)};
    DrawText("Ticket summary", 32, 675, 22, DARKBLUE);
    for (int i = 0; i < 4; ++i) {
        int x = 32 + (i % 2) * 196;
        int y = 705 + (i / 2) * 102;
        DrawRectangle(x, y, 184, 88, WHITE);
        DrawRectangleLines(x, y, 184, 88, LIGHTGRAY);
        DrawText(count_labels[i], x + 12, y + 12, 18, DARKGRAY);
        DrawText(TextFormat("%d", counts[i]), x + 12, y + 40, 30, count_colors[i]);
    }
    for (int line = 0; line < 3 && app->error[line * 45]; ++line)
        DrawText(TextSubtext(app->error, line * 45, 45), 32, 604 + line * 20, 16, MAROON);
    DrawText(TextFormat("Tickets: %d / %d | Ctrl+S to save", (int)app->count, MAX_TICKETS),
             32, 923, 18, DARKGRAY);
}
