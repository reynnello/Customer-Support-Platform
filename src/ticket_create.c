#include "app.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static bool fail(AppState *app, const char *message)
{
    snprintf(app->error, sizeof app->error, "%s", message);
    return false;
}

static bool valid_text(const char *text, size_t capacity)
{
    bool has_content = false;
    if (!text) return false;
    for (size_t i = 0; i < capacity; ++i) {
        if (text[i] == '\0') return has_content;
        if (!isspace((unsigned char)text[i])) has_content = true;
    }
    return false;
}

bool ticket_create(AppState *app, const char *customer,
                   const char *subject, const char *description)
{
    if (!app) return false;
    if (app->count >= MAX_TICKETS) return fail(app, "Ticket capacity reached.");
    if (!valid_text(customer, CUSTOMER_CAP))
        return fail(app, "Customer is required (maximum 63 bytes).");
    if (!valid_text(subject, SUBJECT_CAP))
        return fail(app, "Subject is required (maximum 95 bytes).");
    if (!valid_text(description, DESCRIPTION_CAP))
        return fail(app, "Description is required (maximum 511 bytes).");
    /* Reserve room for next_id, which must remain above every allocated ID. */
    if (app->next_id <= 0 || app->next_id == INT_MAX)
        return fail(app, "Ticket IDs exhausted.");
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id >= app->next_id)
            return fail(app, "Invalid next ticket ID.");

    Ticket ticket = {0};
    ticket.id = app->next_id;
    memcpy(ticket.customer, customer, strlen(customer) + 1);
    memcpy(ticket.subject, subject, strlen(subject) + 1);
    memcpy(ticket.description, description, strlen(description) + 1);
    ticket.status = TICKET_OPEN;
    ticket.priority = PRIORITY_MEDIUM;
    app->tickets[app->count++] = ticket;
    ++app->next_id;
    app->selected_id = ticket.id;
    app->dirty = true;
    app->error[0] = '\0';
    return true;
}

bool ticket_set_status(AppState *app, int id, TicketStatus status)
{
    if (!app) return false;
    if (status < TICKET_OPEN || status > TICKET_RESOLVED)
        return fail(app, "Invalid ticket status.");
    if (id <= 0) return fail(app, "Select a valid ticket.");
    if (app->count > MAX_TICKETS) return fail(app, "Invalid ticket count.");
    for (size_t i = 0; i < app->count; ++i) {
        if (app->tickets[i].id != id) continue;
        if (app->tickets[i].status != status) {
            app->tickets[i].status = status;
            app->dirty = true;
        }
        app->error[0] = '\0';
        return true;
    }
    return fail(app, "Ticket not found.");
}
