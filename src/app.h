#ifndef SUPPORT_APP_H
#define SUPPORT_APP_H

#include <stdbool.h>
#include <stddef.h>

/* Capacities include the terminating NUL byte. IDs start at 1. */
#define MAX_TICKETS 100
#define CUSTOMER_CAP 64
#define SUBJECT_CAP 96
#define DESCRIPTION_CAP 512
#define ASSIGNEE_CAP 64
#define ERROR_CAP 160
#define DEFAULT_DATA_FILE "tickets.txt"

typedef enum {
    TICKET_OPEN,
    TICKET_IN_PROGRESS,
    TICKET_RESOLVED
} TicketStatus;

typedef enum {
    PRIORITY_LOW,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH
} TicketPriority;

typedef struct {
    int id;
    char customer[CUSTOMER_CAP];
    char subject[SUBJECT_CAP];
    char description[DESCRIPTION_CAP];
    char assignee[ASSIGNEE_CAP]; /* Empty means unassigned. */
    TicketStatus status;
    TicketPriority priority;
} Ticket;

typedef struct {
    Ticket tickets[MAX_TICKETS];
    size_t count;
    int next_id;
    int selected_id; /* 0 means no selection. Never store a filtered row index here. */
    bool dirty;     /* Unsaved data changes. Selection alone does not set this. */
    char error[ERROR_CAP];
} AppState;

void app_init(AppState *app);
const char *ticket_status_name(TicketStatus status);
const char *ticket_priority_name(TicketPriority priority);

/* Planned contracts below. Functions are NOT implemented yet.
   Mutations return false and set app->error on failure, leaving ticket data
   unchanged. Success clears error and sets dirty for data changes.
   All pointers must be non-NULL unless explicitly documented otherwise. */

/* Anton: bounded string copies, required customer/subject/description,
   initial OPEN + MEDIUM + unassigned, unique ID, selection of new ticket.
   Fail on full store, invalid/overlong input or exhausted int IDs. */
bool ticket_create(AppState *app, const char *customer,
                   const char *subject, const char *description);
bool ticket_set_status(AppState *app, int id, TicketStatus status);

/* Dzheffrei: query does not mutate state. Empty text matches all; text search
   is ASCII case-insensitive across customer and subject. -1 means any status
   or priority. Other filter values must be valid enum members.
   Write at most capacity IDs, return number written, preserve insertion order.
   out_ids may be NULL only when capacity == 0. */
size_t ticket_query(const AppState *app, const char *text, int status_filter,
                    int priority_filter, int *out_ids, size_t capacity);
bool ticket_set_priority(AppState *app, int id, TicketPriority priority);
bool ticket_set_assignee(AppState *app, int id, const char *assignee);

/* Dzheffrei: save/load return false with error on failure. Save must not
   destroy a previous good file on failure. Successful save clears dirty/error.
   Load validates into temporary state first; failure preserves current data.
   Missing file is an error here; main handles first launch separately.
   Successful load resets selection, clears dirty/error, sets next_id above
   all loaded IDs. Reject duplicate IDs, invalid enums, lengths or capacity.
   File format is to be specified in docs before implementation. */
bool storage_save(AppState *app, const char *path);
bool storage_load(AppState *app, const char *path);

#endif
