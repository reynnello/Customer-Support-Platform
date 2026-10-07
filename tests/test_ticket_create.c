#include "app.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static void unchanged(const AppState *before, const AppState *after)
{
    assert(memcmp(before->tickets, after->tickets, sizeof before->tickets) == 0);
    assert(before->count == after->count);
    assert(before->next_id == after->next_id);
    assert(before->selected_id == after->selected_id);
    assert(before->dirty == after->dirty);
    assert(after->error[0]);
}

int main(void)
{
    AppState app, before;
    app_init(&app);
    before = app;
    assert(!ticket_create(&app, "", "Subject", "Description")); unchanged(&before, &app);
    assert(!ticket_create(&app, " \t", "Subject", "Description")); unchanged(&before, &app);
    assert(!ticket_create(&app, "Customer", "", "Description")); unchanged(&before, &app);
    assert(!ticket_create(&app, "Customer", "Subject", "")); unchanged(&before, &app);
    assert(!ticket_create(&app, NULL, "Subject", "Description")); unchanged(&before, &app);
    char customer[CUSTOMER_CAP + 1], subject[SUBJECT_CAP + 1], description[DESCRIPTION_CAP + 1];
    memset(customer, 'c', sizeof customer); customer[CUSTOMER_CAP] = 0;
    memset(subject, 's', sizeof subject); subject[SUBJECT_CAP] = 0;
    memset(description, 'd', sizeof description); description[DESCRIPTION_CAP] = 0;
    assert(!ticket_create(&app, customer, "s", "d")); unchanged(&before, &app);
    assert(!ticket_create(&app, "c", subject, "d")); unchanged(&before, &app);
    assert(!ticket_create(&app, "c", "s", description)); unchanged(&before, &app);
    customer[CUSTOMER_CAP - 1] = subject[SUBJECT_CAP - 1] = description[DESCRIPTION_CAP - 1] = 0;
    assert(ticket_create(&app, customer, subject, description));
    assert(app.count == 1 && app.next_id == 2 && app.selected_id == 1);
    assert(app.dirty && !app.error[0]);
    assert(!strcmp(app.tickets[0].description, description));
    assert(!strcmp(app.tickets[0].customer, customer));
    assert(!strcmp(app.tickets[0].subject, subject));
    assert(app.tickets[0].status == TICKET_OPEN && app.tickets[0].priority == PRIORITY_MEDIUM);
    assert(!app.tickets[0].assignee[0]);
    app.dirty = false;
    before = app;
    assert(!ticket_set_status(&app, 1, (TicketStatus)99)); unchanged(&before, &app);
    assert(!ticket_set_status(&app, 0, TICKET_OPEN)); unchanged(&before, &app);
    assert(!ticket_set_status(&app, 42, TICKET_OPEN)); unchanged(&before, &app);
    assert(ticket_set_status(&app, 1, TICKET_OPEN));
    assert(!app.dirty && !app.error[0]);
    assert(ticket_set_status(&app, 1, TICKET_IN_PROGRESS));
    assert(app.dirty && app.tickets[0].status == TICKET_IN_PROGRESS);
    assert(ticket_set_status(&app, 1, TICKET_RESOLVED));
    assert(app.tickets[0].status == TICKET_RESOLVED);
    app.next_id = 10;
    assert(ticket_create(&app, "Second", "Request", "Details"));
    assert(ticket_set_status(&app, 10, TICKET_RESOLVED));
    assert(app.tickets[1].status == TICKET_RESOLVED && app.selected_id == 10);
    app.next_id = 10; before = app;
    assert(!ticket_create(&app, "c", "s", "d")); unchanged(&before, &app);
    app.next_id = INT_MAX - 1;
    assert(ticket_create(&app, "c", "s", "d"));
    assert(app.next_id == INT_MAX);
    before = app;
    assert(!ticket_create(&app, "c", "s", "d")); unchanged(&before, &app);
    app_init(&app);
    for (int i = 0; i < MAX_TICKETS; ++i) {
        assert(ticket_create(&app, "c", "s", "d"));
        assert(app.tickets[i].id == i + 1);
    }
    before = app;
    assert(!ticket_create(&app, "c", "s", "d")); unchanged(&before, &app);
    assert(!ticket_create(NULL, "c", "s", "d"));
    assert(!ticket_set_status(NULL, 1, TICKET_OPEN));
    puts("All ticket creation and status tests passed.");
}
