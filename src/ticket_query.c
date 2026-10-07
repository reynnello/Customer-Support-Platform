#include "app.h"

#include <string.h>

/* ASCII-only lowercase; independent of the current C locale. */
static char ascii_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

/* True if needle occurs in haystack, ignoring ASCII case.
   An empty needle matches everything. */
static bool contains_ci(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return true;

    for (const char *h = haystack; *h != '\0'; ++h) {
        size_t i = 0;
        while (i < needle_len && h[i] != '\0' &&
               ascii_lower(h[i]) == ascii_lower(needle[i]))
            ++i;
        if (i == needle_len) return true;
    }
    return false;
}

static bool valid_status_filter(int filter)
{
    return filter == -1 || (filter >= TICKET_OPEN && filter <= TICKET_RESOLVED);
}

static bool valid_priority_filter(int filter)
{
    return filter == -1 || (filter >= PRIORITY_LOW && filter <= PRIORITY_HIGH);
}

size_t ticket_query(const AppState *app, const char *text, int status_filter,
                    int priority_filter, int *out_ids, size_t capacity)
{
    /* Invalid arguments: report no matches instead of crashing. */
    if (app == NULL || text == NULL) return 0;
    if (out_ids == NULL && capacity > 0) return 0;
    if (!valid_status_filter(status_filter)) return 0;
    if (!valid_priority_filter(priority_filter)) return 0;

    size_t count = app->count <= MAX_TICKETS ? app->count : MAX_TICKETS;
    size_t written = 0;

    /* Walk tickets in insertion order; stop once out_ids is full. */
    for (size_t i = 0; i < count && written < capacity; ++i) {
        const Ticket *t = &app->tickets[i];

        if (status_filter != -1 && (int)t->status != status_filter) continue;
        if (priority_filter != -1 && (int)t->priority != priority_filter) continue;
        if (!contains_ci(t->customer, text) && !contains_ci(t->subject, text))
            continue;

        out_ids[written++] = t->id;
    }
    return written;
}
