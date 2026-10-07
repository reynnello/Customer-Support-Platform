#include "app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test helper: append a ticket directly, without ticket_create
   (that function lives in Anton's module). */
static void add(AppState *app, const char *customer, const char *subject,
                TicketStatus status, TicketPriority priority)
{
    Ticket *t = &app->tickets[app->count++];
    *t = (Ticket){0};
    t->id = app->next_id++;
    snprintf(t->customer, sizeof t->customer, "%s", customer);
    snprintf(t->subject, sizeof t->subject, "%s", subject);
    snprintf(t->description, sizeof t->description, "Description");
    t->status = status;
    t->priority = priority;
}

int main(void)
{
    AppState app, before;
    int ids[MAX_TICKETS];
    size_t n;

    app_init(&app);

    /* Empty store returns nothing. */
    assert(ticket_query(&app, "", -1, -1, ids, MAX_TICKETS) == 0);

    add(&app, "Acme Ltd", "Login fails", TICKET_OPEN, PRIORITY_HIGH);          /* id 1 */
    add(&app, "John Smith", "Invoice missing", TICKET_IN_PROGRESS, PRIORITY_MEDIUM); /* id 2 */
    add(&app, "Bob's Cafe", "Printer for ACME receipts", TICKET_RESOLVED, PRIORITY_LOW); /* id 3 */
    add(&app, "acme ltd", "Password reset", TICKET_OPEN, PRIORITY_LOW);        /* id 4 */
    before = app;

    /* Empty text + no filters returns all IDs in insertion order. */
    n = ticket_query(&app, "", -1, -1, ids, MAX_TICKETS);
    assert(n == 4 && ids[0] == 1 && ids[1] == 2 && ids[2] == 3 && ids[3] == 4);

    /* Case-insensitive search across customer and subject. */
    n = ticket_query(&app, "AcMe", -1, -1, ids, MAX_TICKETS);
    assert(n == 3 && ids[0] == 1 && ids[1] == 3 && ids[2] == 4);

    /* Subject only match. */
    n = ticket_query(&app, "invoice", -1, -1, ids, MAX_TICKETS);
    assert(n == 1 && ids[0] == 2);

    /* No match. */
    assert(ticket_query(&app, "zzz", -1, -1, ids, MAX_TICKETS) == 0);

    /* Status filter. */
    n = ticket_query(&app, "", TICKET_OPEN, -1, ids, MAX_TICKETS);
    assert(n == 2 && ids[0] == 1 && ids[1] == 4);

    /* Priority filter. */
    n = ticket_query(&app, "", -1, PRIORITY_LOW, ids, MAX_TICKETS);
    assert(n == 2 && ids[0] == 3 && ids[1] == 4);

    /* Text + status + priority together. */
    n = ticket_query(&app, "acme", TICKET_OPEN, PRIORITY_HIGH, ids, MAX_TICKETS);
    assert(n == 1 && ids[0] == 1);

    /* Capacity smaller than number of matches: writes only capacity IDs. */
    ids[1] = -99;
    n = ticket_query(&app, "", -1, -1, ids, 1);
    assert(n == 1 && ids[0] == 1 && ids[1] == -99);

    /* capacity 0 with NULL out_ids is allowed. */
    assert(ticket_query(&app, "", -1, -1, NULL, 0) == 0);

    /* Invalid arguments return 0. */
    assert(ticket_query(&app, "", 7, -1, ids, MAX_TICKETS) == 0);
    assert(ticket_query(&app, "", -1, -2, ids, MAX_TICKETS) == 0);
    assert(ticket_query(&app, NULL, -1, -1, ids, MAX_TICKETS) == 0);
    assert(ticket_query(NULL, "", -1, -1, ids, MAX_TICKETS) == 0);
    assert(ticket_query(&app, "", -1, -1, NULL, 5) == 0);

    /* Query never modifies state. */
    assert(memcmp(&before, &app, sizeof app) == 0);

    puts("test_ticket_query: all tests passed");
    return 0;
}
