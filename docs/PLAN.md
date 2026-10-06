# Coursework plan

Project: Customer Support Platform. Team: Anton Opria and Dzheffrei Ihesonulo.
Target: a local desktop prototype in C and Raylib, with a scope budget of 15 hours
across three weeks (5 hours per week). This is a planning budget, not recorded work.
Confirm whether the lecturer intends the hours per student or per group.

## Three development weeks

1. Foundation (5 hours): build setup, ticket model, list/detail UI, status changes,
   creation form and required-field validation. First session implements only
   the build, sample data, list/detail UI and status controls.
2. Useful workflow (5 hours): search/filter, priority and assignee, local text-file
   saving/loading with validation and clear error messages.
3. Demo readiness (5 hours): edge cases, usability fixes, code walkthrough,
   screencast and demo rehearsal. Cut optional features before exceeding time.

No server, real email delivery, payments, chatbot or online account system is
needed for this proposed prototype. The brief requires AI-assisted development;
it does not require an AI feature inside the application.

## Deliverables

Every Tuesday, Wednesday and Friday: each member uploads their own development
journal using the supplied Microsoft form; the group records the current product
and stores the screencast in a shared OneDrive folder. Each Tuesday: group demo.
Names alone do not establish individual work; each student must add their own
contribution, hours and reflection before uploading.

The supplied brief says November 3 at 11:00 for the first demo. Assuming the
current academic year, this is November 3, 2026; confirm the year and timezone
with the lecturer. The second 20% assignment and its final demo remain TBA.
The exact three-week start/end dates were not supplied.

## First screencast outline

Show the app, select each sample ticket, change a ticket to In progress and then
Resolved, and show the changing counters. Explain that data resets on restart.
Then show the C Ticket struct and the Raylib drawing loop. Recording and upload
are still to be completed by the group.

## AI convention

Aim for two, maximum three substantive prompts each day. Today has three substantive
requests (initial project work, workflow explanation, and feature allocation/branches); answers identifying OS and names are clarifications. Internal
tool calls are not additional student prompts. This voluntarily limits requests
but does not turn the current tool into a free AI service. The session used Codex;
its subscription/free-tier eligibility has not been verified. To meet the brief,
use an eligible free service or confirm accepted tooling with the lecturer.

## Sources

- Assignment brief supplied in chat on October 6, 2026.
- Project list: https://noelohara.github.io/pointersquiz/advancedprog.html
- Repository: https://github.com/reynnello/Customer-Support-Platform
- Submission form: https://forms.cloud.microsoft/Pages/ResponsePage.aspx?id=OcL4BRfLVUejRQSHUgY8O2ySLwjsnElNlnFmGMhi3aZUMUdYRksyQ0hXNDdEN0U3NDVDWENZTEZGNi4u

## Proposed responsibilities

Anton Opria: Raylib layout, ticket list/detail view, creation form, keyboard and
mouse input, validation messages. Explain the drawing loop and UI input handling.

Dzheffrei Ihesonulo: ticket structs/enums, ID generation, search/filter logic,
file saving/loading and malformed-file handling. Explain arrays, string bounds,
file I/O and the ticket lifecycle.

Both: agree interfaces before editing, review each other's C code, test the
integrated app, understand the full basic flow, record demos and maintain separate
honest journals. These are proposed future responsibilities, not completed work.
Alternate the shared screencast presenter. Each member writes their own reflection.

## Completion criteria

Create a ticket with a unique ID, customer, subject, description, priority and
assignee; reject empty required fields and overlong input safely. Select a ticket,
change status (Open / In progress / Resolved), search and filter it. Save records
locally and reload after restart; report file errors without silently losing data.
Show accurate status counters and offer a clear way to exit. Sample data must be
fictional. These are proposed prototype criteria; the supplied brief does not
prescribe an exact feature list for this topic.

## Working session routine

Agree one small feature and write one focused AI request. Read and explain the
result, build on Linux, test normal and invalid inputs, then save the changes in
Git with a descriptive commit. Record the prompt, changes, actual checks and each
person's own contribution in the journals. On submission days, record a short
shared screencast and upload both personal journals separately. A local commit
must be pushed to GitHub before the other member can pull it.

## Assigned feature branches

- Anton: `codex/anton-features`; creation, list/detail, statuses and UI integration.
- Dzheffrei: `codex/dzheffrei-features`; search/filter, priority/assignee and persistence.

Detailed tasks and completion criteria are in `docs/tasks/anton-opria-functions.docx`
and `docs/tasks/dzheffrei-ihesonulo-functions.docx`. Both start from the same
shared prototype. Agree the shared Ticket header first; Anton owns main/UI and
Dzheffrei supplies separate query, edit and storage modules. Use pull requests
and jointly check the integrated application before merging into main.
