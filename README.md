# Customer Support Platform

Minimal shared foundation for Anton Opria and Dzheffrei Ihesonulo.
**C11 + Raylib 5.5, Linux.** This currently opens an empty window with a ticket
counter. Ticket creation, search, editing and persistence are not implemented.

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
