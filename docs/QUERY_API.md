# Search and filters: how to connect `ticket_query` to the UI

For Anton. Implemented by Dzheffriei in `src/ticket_query.c`; declared in `app.h`.

## The function

```c
size_t ticket_query(const AppState *app, const char *text,
                    int status_filter, int priority_filter,
                    int *out_ids, size_t capacity);
```

| Argument | Meaning |
|---|---|
| `text` | Searched in **customer** and **subject**, ASCII case-insensitive. `""` = no text search. |
| `status_filter` | `-1` = any status, or `TICKET_OPEN` / `TICKET_IN_PROGRESS` / `TICKET_RESOLVED`. |
| `priority_filter` | `-1` = any priority, or `PRIORITY_LOW` / `PRIORITY_MEDIUM` / `PRIORITY_HIGH`. |
| `out_ids` | Array that receives the **IDs** (not indexes) of matching tickets, in insertion order. |
| `capacity` | Size of `out_ids`. Use `MAX_TICKETS` so every match fits. |
| return | Number of IDs written. `0` = nothing matched (or invalid arguments). |

It only reads `app`: it never deletes, reorders or changes tickets, and never touches `selected_id`, `dirty` or `error`.

**Reset filters** = call it with `""`, `-1`, `-1`. That returns every ticket.

## Suggested wiring in `ui.c`

The list currently draws `app->tickets[first_row + row]`. With filtering, draw from the query result instead:

```c
/* UI state for search and filters */
static char search[SUBJECT_CAP];      /* search box text */
static int status_filter = -1;        /* -1 = any */
static int priority_filter = -1;      /* -1 = any */
static int visible_ids[MAX_TICKETS];  /* result of ticket_query */
static size_t visible_count;

static const Ticket *find_ticket(const AppState *app, int id)
{
    for (size_t i = 0; i < app->count; ++i)
        if (app->tickets[i].id == id) return &app->tickets[i];
    return NULL;
}
```

At the **end** of `ui_update` (after create/status changes, so the list is up to date this frame):

```c
visible_count = ticket_query(app, search, status_filter, priority_filter,
                             visible_ids, MAX_TICKETS);
```

Then use `visible_count` instead of `app->count` for scrolling/clamping `first_row`, and for each row:

```c
const Ticket *ticket = find_ticket(app, visible_ids[first_row + row]);
```

Clicking a row stays the same idea: `app->selected_id = ticket->id;`

## Rules worth keeping

- **Keep selection by ID, never by row index.** Row 0 is a different ticket after a filter changes. `app.h` says the same for `selected_id`.
- If the selected ticket is hidden by a filter, it can stay selected (status buttons still work on it). Clearing the selection is also fine, it is a UI choice.
- **Reset `first_row` to 0** whenever search text or a filter changes, otherwise the list may scroll past the end.
- The search box needs its own entry in the active-field logic, so typing into it does not go into the create form.
- Show something like `"No tickets match the filters"` when `visible_count == 0` but `app->count > 0`.

## Ideas for controls (your choice)

- Search box: same style as the form fields, label `Search (customer or subject)`.
- Status filter: a button that cycles `Any → Open → In progress → Resolved → Any`.
- Priority filter: a button that cycles `Any → Low → Medium → High → Any`.
- Reset button: `search[0] = '\0'; status_filter = -1; priority_filter = -1; first_row = 0;`

`ticket_status_name()` and `ticket_priority_name()` from `app.c` give the button labels.

## Test

`tests/test_ticket_query.c` (16 checks). Build and run:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_ticket_query.c src/app.c src/ticket_query.c -o build/test_ticket_query
./build/test_ticket_query
```
