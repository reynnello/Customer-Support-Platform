# Ticket file format

Used by `storage_save` and `storage_load` (`src/storage.c`).
Default file: `tickets.txt` (`DEFAULT_DATA_FILE` in `app.h`).

## Layout

Plain text, one record per line, lines end with `\n`.

```
CSP-TICKETS 1
<id>	<status>	<priority>	<customer>	<subject>	<assignee>	<description>
<id>	<status>	<priority>	<customer>	<subject>	<assignee>	<description>
...
```

- Line 1 is the header and must be exactly `CSP-TICKETS 1` (format version 1).
- Every following line is one ticket: exactly **7 fields separated by one TAB** (6 tabs).
- Tickets are stored in insertion order; loading keeps that order.
- A file with only the header is valid and means zero tickets.

## Fields

| # | Field | Format | Allowed values |
|---|---|---|---|
| 1 | id | decimal digits, no sign, no leading zeros | 1 … INT_MAX − 1, unique in the file |
| 2 | status | word | `OPEN`, `IN_PROGRESS`, `RESOLVED` |
| 3 | priority | word | `LOW`, `MEDIUM`, `HIGH` |
| 4 | customer | escaped text | 1–63 bytes after unescaping, not only whitespace |
| 5 | subject | escaped text | 1–95 bytes after unescaping, not only whitespace |
| 6 | assignee | escaped text | 0–63 bytes; empty = unassigned; not only whitespace |
| 7 | description | escaped text | 1–511 bytes after unescaping, not only whitespace |

Byte limits are the `*_CAP` values in `app.h` minus the terminating NUL.

## Escaping

Text fields may contain characters that would break the layout, so they are escaped on save:

| Character | Written as |
|---|---|
| backslash `\` | `\\` |
| TAB | `\t` |
| newline | `\n` |
| carriage return | `\r` |

On load, any other sequence starting with `\` (for example `\x` or a `\` at the end of a field) is an error.

## Example

```
CSP-TICKETS 1
1	OPEN	HIGH	Acme Ltd	Login fails	Anton	Can't log in\nsince Monday
2	IN_PROGRESS	MEDIUM	John Smith	Invoice missing		September invoice not received
3	RESOLVED	LOW	Bob's Cafe	Printer offline	Dzheffriei	Fixed: driver reinstalled
```

(Ticket 2 has an empty assignee: two TABs in a row.)

## Not stored

`selected_id`, `dirty` and `error` are runtime state and are not saved.
`next_id` is not saved either: after loading it is set to the highest loaded ID + 1 (or 1 for an empty file).

## Saving rules

1. Write everything to a temporary file `<path>.tmp`.
2. Check every write, `fflush` and `fclose` for errors.
3. Only if all of them succeeded, `rename("<path>.tmp", path)` replaces the old file in one step.
4. On any failure: remove `<path>.tmp`, keep the previous file untouched, return `false` and set `app->error`.
5. On success: clear `dirty` and `error`.

## Loading rules

The whole file is parsed into a temporary `AppState` first. The current data is replaced only if the entire file is valid.

The load fails (returns `false`, sets `app->error`, current data unchanged) when:

- the file cannot be opened (a missing file is an error here; `main.c` handles first launch separately);
- the header is missing or different from `CSP-TICKETS 1`;
- a line is empty or longer than 2048 bytes;
- a line does not have exactly 7 fields;
- an ID is not a valid number in range, or appears twice;
- a status or priority word is unknown;
- a text field breaks the length or escaping rules above;
- the file has more than `MAX_TICKETS` (100) tickets;
- a read error occurs.

A `\r` directly before `\n` (Windows line endings) is accepted and ignored.

Error messages include the line number, for example `tickets.txt line 4: unknown priority "URGENT".`

On success: tickets are replaced, `selected_id = 0`, `dirty = false`, `error` cleared, `next_id = max ID + 1`.
