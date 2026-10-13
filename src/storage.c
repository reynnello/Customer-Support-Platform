#include "app.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Format: see docs/FILE_FORMAT.md */
#define FILE_HEADER "CSP-TICKETS 1"
#define FIELD_COUNT 7
#define MAX_LINE 2048      /* longest accepted line, without the newline */
#define MAX_PATH_LEN 1024  /* longest accepted file path */

static const char *const STATUS_WORDS[] = {"OPEN", "IN_PROGRESS", "RESOLVED"};
static const char *const PRIORITY_WORDS[] = {"LOW", "MEDIUM", "HIGH"};

static bool fail(AppState *app, const char *message)
{
    snprintf(app->error, sizeof app->error, "%s", message);
    return false;
}

/* ------------------------------------------------------------------ */
/* Saving                                                              */
/* ------------------------------------------------------------------ */

/* Write text with \\, \t, \n and \r escaped. Returns false on write error. */
static bool write_escaped(FILE *f, const char *text)
{
    for (const char *p = text; *p != '\0'; ++p) {
        int ok;
        switch (*p) {
            case '\\': ok = fputs("\\\\", f); break;
            case '\t': ok = fputs("\\t", f); break;
            case '\n': ok = fputs("\\n", f); break;
            case '\r': ok = fputs("\\r", f); break;
            default:   ok = fputc(*p, f); break;
        }
        if (ok == EOF) return false;
    }
    return true;
}

static bool write_ticket(FILE *f, const Ticket *t)
{
    if (fprintf(f, "%d\t%s\t%s\t", t->id, STATUS_WORDS[t->status],
                PRIORITY_WORDS[t->priority]) < 0) return false;
    return write_escaped(f, t->customer) && fputc('\t', f) != EOF &&
           write_escaped(f, t->subject) && fputc('\t', f) != EOF &&
           write_escaped(f, t->assignee) && fputc('\t', f) != EOF &&
           write_escaped(f, t->description) && fputc('\n', f) != EOF;
}

bool storage_save(AppState *app, const char *path)
{
    if (app == NULL) return false;
    if (path == NULL || path[0] == '\0') return fail(app, "Save failed: no file name.");
    if (app->count > MAX_TICKETS) return fail(app, "Save failed: invalid ticket count.");
    for (size_t i = 0; i < app->count; ++i) {
        const Ticket *t = &app->tickets[i];
        if (t->status < TICKET_OPEN || t->status > TICKET_RESOLVED ||
            t->priority < PRIORITY_LOW || t->priority > PRIORITY_HIGH)
            return fail(app, "Save failed: a ticket has an invalid status or priority.");
    }

    /* Write to "<path>.tmp" first; the real file is replaced only at the end. */
    char tmp_path[MAX_PATH_LEN + 8];
    if (strlen(path) > MAX_PATH_LEN) return fail(app, "Save failed: file name is too long.");
    snprintf(tmp_path, sizeof tmp_path, "%s.tmp", path);

    FILE *f = fopen(tmp_path, "w");
    if (f == NULL) {
        snprintf(app->error, sizeof app->error, "Save failed: cannot create %.80s (%s).",
                 tmp_path, strerror(errno));
        return false;
    }

    bool ok = fprintf(f, "%s\n", FILE_HEADER) >= 0;
    for (size_t i = 0; ok && i < app->count; ++i) ok = write_ticket(f, &app->tickets[i]);
    if (fflush(f) == EOF || ferror(f)) ok = false;
    if (fclose(f) == EOF) ok = false;

    if (!ok) {
        remove(tmp_path);
        snprintf(app->error, sizeof app->error, "Save failed: error while writing %.80s.", tmp_path);
        return false;
    }
    if (rename(tmp_path, path) != 0) {
        int saved_errno = errno;
        remove(tmp_path);
        snprintf(app->error, sizeof app->error, "Save failed: cannot replace %.80s (%s).",
                 path, strerror(saved_errno));
        return false;
    }

    app->dirty = false;
    app->error[0] = '\0';
    return true;
}

/* ------------------------------------------------------------------ */
/* Loading                                                             */
/* ------------------------------------------------------------------ */

/* Parse a positive decimal ID: digits only, no leading zero, 1..INT_MAX-1
   (next_id = max + 1 must still fit in an int). */
static bool parse_id(const char *text, int *out)
{
    if (text[0] < '1' || text[0] > '9') return false;
    long long value = 0;
    for (const char *p = text; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') return false;
        value = value * 10 + (*p - '0');
        if (value >= INT_MAX) return false;
    }
    *out = (int)value;
    return true;
}

static int parse_word(const char *text, const char *const words[], int count)
{
    for (int i = 0; i < count; ++i)
        if (strcmp(text, words[i]) == 0) return i;
    return -1;
}

/* Unescape src into dst (capacity cap). Returns NULL on success or a reason. */
static const char *unescape(const char *src, char *dst, size_t cap)
{
    size_t len = 0;
    for (const char *p = src; *p != '\0'; ++p) {
        char c = *p;
        if (c == '\\') {
            ++p;
            switch (*p) {
                case '\\': c = '\\'; break;
                case 't':  c = '\t'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                default:   return "invalid escape sequence";
            }
        }
        if (len + 1 >= cap) return "too long";
        dst[len++] = c;
    }
    dst[len] = '\0';
    return NULL;
}

static bool only_whitespace(const char *text)
{
    for (const char *p = text; *p != '\0'; ++p)
        if (!isspace((unsigned char)*p)) return false;
    return true;
}

/* Decode one text field. required = must not be empty. */
static const char *read_text(const char *src, char *dst, size_t cap, bool required,
                             const char *name, char *why, size_t why_cap)
{
    const char *problem = unescape(src, dst, cap);
    if (problem == NULL && required && dst[0] == '\0') problem = "empty";
    if (problem == NULL && dst[0] != '\0' && only_whitespace(dst)) problem = "only spaces";
    if (problem == NULL) return NULL;
    snprintf(why, why_cap, "%s: %s", name, problem);
    return why;
}

/* Parse one record line (already without the newline) into *t.
   Returns NULL on success, otherwise a short description of the problem. */
static const char *parse_record(char *line, Ticket *t, char *why, size_t why_cap)
{
    char *fields[FIELD_COUNT];
    int n = 0;
    fields[n++] = line;
    for (char *p = line; *p != '\0'; ++p) {
        if (*p != '\t') continue;
        if (n == FIELD_COUNT) return "too many fields (expected 7)";
        *p = '\0';
        fields[n++] = p + 1;
    }
    if (n != FIELD_COUNT) return "too few fields (expected 7)";

    *t = (Ticket){0};
    if (!parse_id(fields[0], &t->id)) {
        snprintf(why, why_cap, "invalid ticket ID \"%.20s\"", fields[0]);
        return why;
    }
    int status = parse_word(fields[1], STATUS_WORDS, 3);
    if (status < 0) {
        snprintf(why, why_cap, "unknown status \"%.20s\"", fields[1]);
        return why;
    }
    int priority = parse_word(fields[2], PRIORITY_WORDS, 3);
    if (priority < 0) {
        snprintf(why, why_cap, "unknown priority \"%.20s\"", fields[2]);
        return why;
    }
    t->status = (TicketStatus)status;
    t->priority = (TicketPriority)priority;

    const char *problem;
    if ((problem = read_text(fields[3], t->customer, CUSTOMER_CAP, true, "customer", why, why_cap)))
        return problem;
    if ((problem = read_text(fields[4], t->subject, SUBJECT_CAP, true, "subject", why, why_cap)))
        return problem;
    if ((problem = read_text(fields[5], t->assignee, ASSIGNEE_CAP, false, "assignee", why, why_cap)))
        return problem;
    if ((problem = read_text(fields[6], t->description, DESCRIPTION_CAP, true, "description", why, why_cap)))
        return problem;
    return NULL;
}

/* Read one line into buf (newline and a preceding \r removed).
   Returns 1 = line read, 0 = end of file, -1 = line too long, -2 = read error. */
static int read_line(FILE *f, char *buf, size_t cap)
{
    if (fgets(buf, (int)cap, f) == NULL) return ferror(f) ? -2 : 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[--len] = '\0';
    } else if (!feof(f)) {
        return -1; /* buffer full before the newline */
    }
    if (len > 0 && buf[len - 1] == '\r') buf[--len] = '\0';
    return 1;
}

bool storage_load(AppState *app, const char *path)
{
    if (app == NULL) return false;
    if (path == NULL || path[0] == '\0') return fail(app, "Load failed: no file name.");

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        snprintf(app->error, sizeof app->error, "Load failed: cannot open %.80s (%s).",
                 path, strerror(errno));
        return false;
    }

    /* Parse into a temporary state; app is touched only after full success. */
    AppState *loaded = malloc(sizeof *loaded);
    if (loaded == NULL) {
        fclose(f);
        return fail(app, "Load failed: out of memory.");
    }
    app_init(loaded);

    char line[MAX_LINE + 2]; /* + newline + NUL */
    char why[96] = "";
    const char *problem = NULL;
    int line_no = 1;
    int max_id = 0;

    int r = read_line(f, line, sizeof line);
    if (r == 0) problem = "file is empty (missing header)";
    else if (r == -1) problem = "header line is too long";
    else if (r == -2) problem = "read error";
    else if (strcmp(line, FILE_HEADER) != 0) problem = "missing or unsupported header";

    while (problem == NULL) {
        r = read_line(f, line, sizeof line);
        ++line_no;
        if (r == 0) break;
        if (r == -1) { problem = "line is too long"; break; }
        if (r == -2) { problem = "read error"; break; }
        if (line[0] == '\0') { problem = "empty line"; break; }
        if (loaded->count == MAX_TICKETS) { problem = "too many tickets (maximum 100)"; break; }

        Ticket t;
        problem = parse_record(line, &t, why, sizeof why);
        if (problem != NULL) break;

        for (size_t i = 0; i < loaded->count; ++i) {
            if (loaded->tickets[i].id == t.id) {
                snprintf(why, sizeof why, "duplicate ticket ID %d", t.id);
                problem = why;
                break;
            }
        }
        if (problem != NULL) break;

        loaded->tickets[loaded->count++] = t;
        if (t.id > max_id) max_id = t.id;
    }
    fclose(f);

    if (problem != NULL) {
        snprintf(app->error, sizeof app->error, "Load failed: %.40s line %d: %.80s.",
                 path, line_no, problem);
        free(loaded);
        return false;
    }

    loaded->next_id = max_id + 1;
    loaded->selected_id = 0;
    loaded->dirty = false;
    loaded->error[0] = '\0';
    *app = *loaded;
    free(loaded);
    return true;
}
