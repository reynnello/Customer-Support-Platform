# Customer Support Platform

A small desktop support-ticket prototype written in **C11 and Raylib 5.5**.
Coursework group: Anton Opria and Dzheffrei Ihesonulo.

## Session 1

- Three fictional sample tickets with customer, subject and description.
- Click a ticket to inspect it.
- Click Open, In progress or Resolved to set its status.
- Summary counters reflect the current statuses.
- Escape or the window close button exits.

Data is currently held in memory and resets on restart. Ticket creation,
search, persistence and assignment are planned; they are not implemented yet.

## Build on Linux

Install a C compiler, CMake, Git and the X11/OpenGL development libraries.
For Debian/Ubuntu, the prerequisite packages are:

```sh
sudo apt install build-essential cmake git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
cmake -S . -B build
cmake --build build -j
./build/customer_support
```

CMake uses installed Raylib 5.5 when available, otherwise downloads the pinned
5.5 release from its official repository. The first configure needs internet.
Run from a graphical Linux desktop. Application source is C, not C++.

## Coursework workflow

See [the project plan](docs/PLAN.md), [prompt log](docs/journal/2026-10-06.md)
and the individual Word journals in `docs/journal/`.
The journals require each student's own contribution, actual hours and reflection
before submission. Keep one shared screencast per group in OneDrive.
