# Customer Support Platform

Minimal shared foundation for Anton Opria and Dzheffrei Ihesonulo.
**C11 + Raylib 5.5, Linux.** Anton's branch implements ticket creation and
status changes, with a form and selectable ticket list. Search, priority and
assignee editing, and persistence are not implemented.

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
Enter. Use Tab to move between fields. Select a ticket in the list and click
Open, In progress or Resolved to change its status. Scroll over the list to
see more tickets. The initial form supports printable English ASCII input.
The selected ticket card shows ID, customer, subject, description, status,
priority and assignee (Unassigned if empty). Long text wraps; scroll inside
the card to read all details. Changing selection resets the card scroll. Tickets exist only for the
current session and are lost when the program closes.

## Test creation and status changes

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Isrc tests/test_ticket_create.c src/app.c src/ticket_create.c -o build/test_ticket_create
./build/test_ticket_create
```

The ticket summary displays Total tickets, Open, In progress and Resolved.
Counts refresh immediately after creation or a status change and include all
tickets, including those outside the visible list.
