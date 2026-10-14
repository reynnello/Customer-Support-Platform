#include "app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test helper: append a ticket directly (ticket_create is Anton's module). */
static void add(AppState *app, const char *customer, const char *subject)
{
    Ticket *t = &app->tickets[app->count++];
    *t = (Ticket){0};
    t->id = app->next_id++;
    snprintf(t->customer, sizeof t->customer, "%s", customer);
    snprintf(t->subject, sizeof t->subject, "%s", subject);
    snprintf(t->description, sizeof t->description, "Description");
    t->status = TICKET_OPEN;
    t->priority = PRIORITY_MEDIUM;
}

/* A failed call must keep all ticket data, dirty flag and selection,
   and must set an error message. */
static void unchanged(const AppState *before, const AppState *after)
{
    assert(memcmp(before->tickets, after->tickets, sizeof before->tickets) == 0);
    assert(before->count == after->count);
    assert(before->next_id == after->next_id);
    assert(before->selected_id == after->selected_id);
    assert(before->dirty == after->dirty);
    assert(after->error[0] != '\0');
}

static void test_priority(void)
{
    AppState app, before;
    app_init(&app);
    add(&app, "Acme Ltd", "Login fails");    /* id 1 */
    add(&app, "John Smith", "Invoice");      /* id 2 */

    /* Valid change updates only the target ticket and sets dirty. */
    assert(ticket_set_priority(&app, 2, PRIORITY_HIGH));
    assert(app.tickets[1].priority == PRIORITY_HIGH);
    assert(app.tickets[0].priority == PRIORITY_MEDIUM);
    assert(app.dirty);
    assert(app.error[0] == '\0');

    /* Setting the same value succeeds but is not a data change. */
    app.dirty = false;
    assert(ticket_set_priority(&app, 2, PRIORITY_HIGH));
    assert(!app.dirty);

    /* All three values are accepted. */
    assert(ticket_set_priority(&app, 1, PRIORITY_LOW));
    assert(app.tickets[0].priority == PRIORITY_LOW);

    /* Failures leave everything unchanged. */
    before = app;
    assert(!ticket_set_priority(&app, 99, PRIORITY_HIGH)); unchanged(&before, &app);
    assert(!ticket_set_priority(&app, 0, PRIORITY_HIGH)); unchanged(&before, &app);
    assert(!ticket_set_priority(&app, -1, PRIORITY_HIGH)); unchanged(&before, &app);
    assert(!ticket_set_priority(&app, 1, (TicketPriority)3)); unchanged(&before, &app);
    assert(!ticket_set_priority(&app, 1, (TicketPriority)-1)); unchanged(&before, &app);
    assert(!ticket_set_priority(NULL, 1, PRIORITY_HIGH));

    /* A success after a failure clears the error. */
    assert(ticket_set_priority(&app, 1, PRIORITY_MEDIUM));
    assert(app.error[0] == '\0');
}

static void test_assignee(void)
{
    AppState app, before;
    app_init(&app);
    add(&app, "Acme Ltd", "Login fails");    /* id 1 */
    add(&app, "John Smith", "Invoice");      /* id 2 */

    /* Assign, reassign, unassign. */
    assert(ticket_set_assignee(&app, 1, "Anton"));
    assert(strcmp(app.tickets[0].assignee, "Anton") == 0);
    assert(app.tickets[1].assignee[0] == '\0');
    assert(app.dirty);

    assert(ticket_set_assignee(&app, 1, "Dzheffriei"));
    assert(strcmp(app.tickets[0].assignee, "Dzheffriei") == 0);

    assert(ticket_set_assignee(&app, 1, ""));
    assert(app.tickets[0].assignee[0] == '\0');

    /* Same value: success, no dirty. */
    app.dirty = false;
    assert(ticket_set_assignee(&app, 1, ""));
    assert(!app.dirty);

    /* Boundary: exactly 63 bytes fits, 64 does not. */
    char name[ASSIGNEE_CAP + 1];
    memset(name, 'a', ASSIGNEE_CAP - 1);
    name[ASSIGNEE_CAP - 1] = '\0';
    assert(ticket_set_assignee(&app, 2, name));
    assert(strlen(app.tickets[1].assignee) == ASSIGNEE_CAP - 1);

    memset(name, 'b', ASSIGNEE_CAP);
    name[ASSIGNEE_CAP] = '\0';
    before = app;
    assert(!ticket_set_assignee(&app, 2, name)); unchanged(&before, &app);

    /* Invalid input and unknown tickets. */
    assert(!ticket_set_assignee(&app, 1, "   ")); unchanged(&before, &app);
    assert(!ticket_set_assignee(&app, 1, "Bad\tName")); unchanged(&before, &app);
    assert(!ticket_set_assignee(&app, 1, "Bad\nName")); unchanged(&before, &app);
    assert(!ticket_set_assignee(&app, 1, NULL)); unchanged(&before, &app);
    assert(!ticket_set_assignee(&app, 99, "Anton")); unchanged(&before, &app);
    assert(!ticket_set_assignee(&app, 0, "Anton")); unchanged(&before, &app);
    assert(!ticket_set_assignee(NULL, 1, "Anton"));

    /* Names with inner spaces are fine. */
    assert(ticket_set_assignee(&app, 1, "Anton Opria"));
    assert(strcmp(app.tickets[0].assignee, "Anton Opria") == 0);
}

int main(void)
{
    test_priority();
    test_assignee();
    puts("test_ticket_edit: all tests passed");
    return 0;
}
