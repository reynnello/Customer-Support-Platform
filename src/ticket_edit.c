#include "app.h"

#include <stdio.h>
#include <string.h>

/* Set the error message and report failure. Ticket data is not touched. */
static bool fail(AppState *app, const char *message)
{
    snprintf(app->error, sizeof app->error, "%s", message);
    return false;
}

/* Find a ticket by ID. Returns NULL if the ID does not exist. */
static Ticket *find_ticket(AppState *app, int id)
{
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id == id) return &app->tickets[i];
    return NULL;
}

/* Common checks for both mutations: valid state and an existing ticket. */
static Ticket *find_for_edit(AppState *app, int id)
{
    if (app->count > MAX_TICKETS) {
        fail(app, "Invalid ticket count.");
        return NULL;
    }
    if (id <= 0) {
        fail(app, "Select a valid ticket.");
        return NULL;
    }
    Ticket *ticket = find_ticket(app, id);
    if (ticket == NULL) fail(app, "Ticket not found.");
    return ticket;
}

bool ticket_set_priority(AppState *app, int id, TicketPriority priority)
{
    if (app == NULL) return false;
    if (priority < PRIORITY_LOW || priority > PRIORITY_HIGH)
        return fail(app, "Invalid ticket priority.");

    Ticket *ticket = find_for_edit(app, id);
    if (ticket == NULL) return false;

    if (ticket->priority != priority) {
        ticket->priority = priority;
        app->dirty = true; /* only a real change counts as unsaved data */
    }
    app->error[0] = '\0';
    return true;
}

/* Assignee rules: at most ASSIGNEE_CAP - 1 bytes, no control characters,
   and not only spaces. An empty string means "unassigned". */
static const char *assignee_problem(const char *assignee)
{
    bool has_content = false;
    for (size_t i = 0; i < ASSIGNEE_CAP; ++i) {
        unsigned char c = (unsigned char)assignee[i];
        if (c == '\0') {
            if (i > 0 && !has_content) return "Assignee cannot be only spaces.";
            return NULL; /* valid, including empty */
        }
        if (c < 32 || c == 127) return "Assignee contains invalid characters.";
        if (c != ' ') has_content = true;
    }
    return "Assignee name is too long (maximum 63 bytes).";
}

bool ticket_set_assignee(AppState *app, int id, const char *assignee)
{
    if (app == NULL) return false;
    if (assignee == NULL) return fail(app, "Assignee is missing.");

    const char *problem = assignee_problem(assignee);
    if (problem != NULL) return fail(app, problem);

    Ticket *ticket = find_for_edit(app, id);
    if (ticket == NULL) return false;

    if (strcmp(ticket->assignee, assignee) != 0) {
        /* Length was checked above, so this copy always fits. */
        memcpy(ticket->assignee, assignee, strlen(assignee) + 1);
        app->dirty = true;
    }
    app->error[0] = '\0';
    return true;
}
