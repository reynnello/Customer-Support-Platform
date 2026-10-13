/* POSIX is used only by this test (mkdir/rmdir); storage.c itself is plain C11. */
#define _POSIX_C_SOURCE 200809L

#include "app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Test files live in build/, which is ignored by git. */
#define PATH "build/test_storage.txt"
#define TMP_PATH PATH ".tmp"

static void add(AppState *app, int id, const char *customer, const char *subject,
                const char *assignee, const char *description,
                TicketStatus status, TicketPriority priority)
{
    Ticket *t = &app->tickets[app->count++];
    *t = (Ticket){0};
    t->id = id;
    snprintf(t->customer, sizeof t->customer, "%s", customer);
    snprintf(t->subject, sizeof t->subject, "%s", subject);
    snprintf(t->assignee, sizeof t->assignee, "%s", assignee);
    snprintf(t->description, sizeof t->description, "%s", description);
    t->status = status;
    t->priority = priority;
    if (id >= app->next_id) app->next_id = id + 1;
}

static void write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    assert(f != NULL);
    fputs(content, f);
    fclose(f);
}

static void read_file(const char *path, char *buf, size_t cap)
{
    FILE *f = fopen(path, "r");
    assert(f != NULL);
    size_t n = fread(buf, 1, cap - 1, f);
    buf[n] = '\0';
    fclose(f);
}

static bool same_ticket(const Ticket *a, const Ticket *b)
{
    return a->id == b->id && a->status == b->status && a->priority == b->priority &&
           strcmp(a->customer, b->customer) == 0 && strcmp(a->subject, b->subject) == 0 &&
           strcmp(a->assignee, b->assignee) == 0 && strcmp(a->description, b->description) == 0;
}

/* State with one known ticket, used to prove that failed loads change nothing. */
static void sample_state(AppState *app)
{
    app_init(app);
    add(app, 1, "Keep me", "Original", "", "Must survive", TICKET_OPEN, PRIORITY_LOW);
    app->selected_id = 1;
    app->dirty = true;
}

static void expect_load_fails(const char *content, const char *expected_text)
{
    AppState app, before;
    sample_state(&app);
    before = app;
    write_file(PATH, content);
    assert(!storage_load(&app, PATH));
    assert(memcmp(&before.tickets, &app.tickets, sizeof app.tickets) == 0);
    assert(app.count == before.count && app.next_id == before.next_id);
    assert(app.selected_id == before.selected_id && app.dirty == before.dirty);
    if (strstr(app.error, expected_text) == NULL) {
        fprintf(stderr, "expected \"%s\" in error, got: %s\n", expected_text, app.error);
        assert(0);
    }
}

static void test_round_trip(void)
{
    AppState app, loaded;
    app_init(&app);

    char long_desc[DESCRIPTION_CAP];
    memset(long_desc, 'd', DESCRIPTION_CAP - 1);
    long_desc[DESCRIPTION_CAP - 1] = '\0';

    add(&app, 5, "Acme Ltd", "Login fails", "Anton", "Can't log in\nsince Monday",
        TICKET_OPEN, PRIORITY_HIGH);
    add(&app, 2, "Tab\tCustomer", "Back\\slash", "", "CR\r and tab\t inside",
        TICKET_IN_PROGRESS, PRIORITY_MEDIUM);
    add(&app, 9, "Bob's Cafe", "Long text", "Dzheffriei", long_desc,
        TICKET_RESOLVED, PRIORITY_LOW);
    app.dirty = true;

    assert(storage_save(&app, PATH));
    assert(!app.dirty && app.error[0] == '\0');

    /* The temporary file must not be left behind. */
    assert(access(TMP_PATH, F_OK) != 0);

    /* Human-readable layout as documented. */
    char text[4096];
    read_file(PATH, text, sizeof text);
    assert(strncmp(text, "CSP-TICKETS 1\n5\tOPEN\tHIGH\tAcme Ltd\tLogin fails\tAnton\t"
                         "Can't log in\\nsince Monday\n", 75) == 0);

    sample_state(&loaded);
    assert(storage_load(&loaded, PATH));
    assert(loaded.count == 3);
    for (size_t i = 0; i < 3; ++i) assert(same_ticket(&app.tickets[i], &loaded.tickets[i]));
    assert(loaded.next_id == 10);           /* max ID 9 + 1 */
    assert(loaded.selected_id == 0);
    assert(!loaded.dirty && loaded.error[0] == '\0');
}

static void test_empty_and_line_endings(void)
{
    AppState app;

    /* Header only = zero tickets. */
    write_file(PATH, "CSP-TICKETS 1\n");
    sample_state(&app);
    assert(storage_load(&app, PATH));
    assert(app.count == 0 && app.next_id == 1);

    /* Saving an empty state works too. */
    assert(storage_save(&app, PATH));
    assert(storage_load(&app, PATH) && app.count == 0);

    /* Windows line endings and a missing final newline are accepted. */
    write_file(PATH, "CSP-TICKETS 1\r\n3\tOPEN\tLOW\tA\tB\t\tC\r\n4\tOPEN\tLOW\tD\tE\t\tF");
    assert(storage_load(&app, PATH));
    assert(app.count == 2 && app.tickets[1].id == 4 && strcmp(app.tickets[1].description, "F") == 0);
    assert(app.next_id == 5);
}

static void test_corrupted_files(void)
{
    expect_load_fails("", "missing header");
    expect_load_fails("TICKETS\n", "unsupported header");
    expect_load_fails("CSP-TICKETS 2\n", "unsupported header");
    expect_load_fails("CSP-TICKETS 1\n\n", "line 2: empty line");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\tC\n", "too few fields");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t\tC\tX\n", "too many fields");
    expect_load_fails("CSP-TICKETS 1\n0\tOPEN\tLOW\tA\tB\t\tC\n", "invalid ticket ID");
    expect_load_fails("CSP-TICKETS 1\n01\tOPEN\tLOW\tA\tB\t\tC\n", "invalid ticket ID");
    expect_load_fails("CSP-TICKETS 1\n-3\tOPEN\tLOW\tA\tB\t\tC\n", "invalid ticket ID");
    expect_load_fails("CSP-TICKETS 1\nabc\tOPEN\tLOW\tA\tB\t\tC\n", "invalid ticket ID");
    expect_load_fails("CSP-TICKETS 1\n2147483647\tOPEN\tLOW\tA\tB\t\tC\n", "invalid ticket ID");
    expect_load_fails("CSP-TICKETS 1\n1\tCLOSED\tLOW\tA\tB\t\tC\n", "unknown status");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tURGENT\tA\tB\t\tC\n", "unknown priority");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\t\tB\t\tC\n", "customer: empty");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\t   \t\tC\n", "subject: only spaces");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t  \tC\n", "assignee: only spaces");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t\t\n", "description: empty");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\\x\tB\t\tC\n", "invalid escape");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\\\tB\t\tC\n", "customer: invalid escape");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t\tC\\\n", "invalid escape");
    expect_load_fails("CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t\tC\n1\tOPEN\tLOW\tD\tE\t\tF\n",
                      "line 3: duplicate ticket ID 1");

    /* Customer of 64 bytes (limit is 63). */
    char buf[8192];
    char name[CUSTOMER_CAP + 1];
    memset(name, 'c', CUSTOMER_CAP);
    name[CUSTOMER_CAP] = '\0';
    snprintf(buf, sizeof buf, "CSP-TICKETS 1\n1\tOPEN\tLOW\t%s\tB\t\tC\n", name);
    expect_load_fails(buf, "customer: too long");

    /* A line longer than 2048 bytes. */
    char huge[3000];
    memset(huge, 'x', sizeof huge - 1);
    huge[sizeof huge - 1] = '\0';
    snprintf(buf, sizeof buf, "CSP-TICKETS 1\n1\tOPEN\tLOW\tA\tB\t\t%s\n", huge);
    expect_load_fails(buf, "line is too long");

    /* 101 tickets: one more than MAX_TICKETS. */
    FILE *f = fopen(PATH, "w");
    assert(f != NULL);
    fputs("CSP-TICKETS 1\n", f);
    for (int id = 1; id <= MAX_TICKETS + 1; ++id) fprintf(f, "%d\tOPEN\tLOW\tA\tB\t\tC\n", id);
    fclose(f);
    AppState app, before;
    sample_state(&app);
    before = app;
    assert(!storage_load(&app, PATH));
    assert(strstr(app.error, "too many tickets") != NULL);
    assert(app.count == before.count && same_ticket(&app.tickets[0], &before.tickets[0]));

    /* Exactly 100 tickets load fine. */
    f = fopen(PATH, "w");
    assert(f != NULL);
    fputs("CSP-TICKETS 1\n", f);
    for (int id = 1; id <= MAX_TICKETS; ++id) fprintf(f, "%d\tOPEN\tLOW\tA\tB\t\tC\n", id);
    fclose(f);
    assert(storage_load(&app, PATH) && app.count == MAX_TICKETS && app.next_id == MAX_TICKETS + 1);
}

static void test_missing_file(void)
{
    AppState app, before;
    sample_state(&app);
    before = app;
    remove("build/does_not_exist.txt");
    assert(!storage_load(&app, "build/does_not_exist.txt"));
    assert(strstr(app.error, "cannot open") != NULL);
    assert(app.count == before.count && app.dirty == before.dirty);
}

static void test_failed_save_keeps_old_file(void)
{
    AppState app;
    sample_state(&app);
    assert(storage_save(&app, PATH));
    char good[4096];
    read_file(PATH, good, sizeof good);

    /* Make "<path>.tmp" impossible to create by putting a directory there. */
    add(&app, 2, "New", "Unsaved", "", "Change", TICKET_OPEN, PRIORITY_HIGH);
    app.dirty = true;
    assert(mkdir(TMP_PATH, 0700) == 0);
    assert(!storage_save(&app, PATH));
    assert(app.dirty);                         /* still unsaved */
    assert(strstr(app.error, "Save failed") != NULL);
    rmdir(TMP_PATH);

    char after[4096];
    read_file(PATH, after, sizeof after);
    assert(strcmp(good, after) == 0);          /* previous good file untouched */

    /* Directory that does not exist. */
    assert(!storage_save(&app, "build/no_such_dir/tickets.txt"));
    assert(app.dirty);

    /* Invalid enum in memory is refused instead of writing a broken file. */
    app.tickets[0].priority = (TicketPriority)7;
    assert(!storage_save(&app, PATH));
    read_file(PATH, after, sizeof after);
    assert(strcmp(good, after) == 0);
}

static void test_null_arguments(void)
{
    AppState app;
    app_init(&app);
    assert(!storage_save(NULL, PATH));
    assert(!storage_load(NULL, PATH));
    assert(!storage_save(&app, NULL) && app.error[0] != '\0');
    assert(!storage_load(&app, NULL) && app.error[0] != '\0');
    assert(!storage_save(&app, "") && !storage_load(&app, ""));
}

int main(void)
{
    test_round_trip();
    test_empty_and_line_endings();
    test_corrupted_files();
    test_missing_file();
    test_failed_save_keeps_old_file();
    test_null_arguments();
    remove(PATH);
    puts("test_storage: all tests passed");
    return 0;
}
