# Customer Support Platform

Coursework prototype by Anton Opria and Dzheffrei Ihesonulo.
**C11 + Raylib 5.5, Linux.** The application supports ticket creation,
search and filters, status and priority changes, assignment, and local file
storage.

Read `src/app.h` and `AGENTS.md`
before asking an AI to implement a feature. The common function names and data
structures must remain compatible across both personal branches.

## Build

Requires C compiler, CMake, Git and X11/OpenGL development libraries.
CMake downloads Raylib 5.5 if it is not already installed.

```sh
cmake -S . -B build
cmake --build build
./build/customer_support
```

## Get the shared foundation in your branch

Start from a clean working tree; commit your own work first if necessary.

Anton:
```sh
git fetch origin
git switch anton
git merge origin/main
```

Dzheffrei:
```sh
git fetch origin
git switch dzheffrei
git merge origin/main
```

If the branch only exists remotely, use `git switch --track origin/anton`
(or `origin/dzheffrei`) instead of the switch command above. Develop and push
in your personal branch, then open a pull request into main.

## Try the implemented features

Enter a customer, subject and description, then click Create ticket or press
Enter while the creation form is active. Use Clear form to discard those three
input fields without changing existing tickets. Use Tab to move between fields.
The search field matches customer and subject text. Use the Status and Priority
filter buttons to cycle through values, or Reset to show all tickets. Scroll
over the list to see more results. Selection stays on the same ticket ID when a
filter hides it. The form supports printable English ASCII input.

Select a ticket in the list, then click Open, In progress or Resolved to change
its status. The Low, Medium and High buttons change its priority. Enter a name
in Assigned to and click Assign (or press Enter in that field); leave it empty
and assign to remove the current name.
The selected ticket card shows ID, customer, subject, description, status,
priority and assignee (Unassigned if empty). Long text wraps; scroll inside
the card to read all details. Changing selection resets the card scroll.

Click Save or press Ctrl+S to write `tickets.txt` in the directory from which
the program was launched. Unsaved changes are also saved on exit. On startup,
an existing file is loaded automatically. If it is invalid, the error is
displayed and the in-memory tickets remain unchanged. Automatic saving is
disabled for that run to avoid overwriting the invalid file; an explicit save
still works. The file format is described in `docs/FILE_FORMAT.md`.

## Run tests

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_ticket_create.c src/app.c src/ticket_create.c -o build/test_ticket_create
./build/test_ticket_create
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_ticket_query.c src/app.c src/ticket_query.c -o build/test_ticket_query
./build/test_ticket_query
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_ticket_edit.c src/app.c src/ticket_edit.c -o build/test_ticket_edit
./build/test_ticket_edit
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_storage.c src/app.c src/storage.c -o build/test_storage
./build/test_storage
```

The ticket summary displays Total tickets, Open, In progress and Resolved.
Counts refresh immediately after creation or a status change and include all
tickets, including those outside the visible list.

Status colors are consistent across the card, list, buttons and counters:
Open is blue, In progress is dark orange, and Resolved is green. The active
status button has a tinted background and thick border. Controls are disabled
until a ticket is selected.
