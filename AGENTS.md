# Shared instructions for both AI assistants

Read README.md and src/app.h before editing.
Use C11 and Raylib 5.5 only; no C++, external GUI framework or backend.
Keep this a small coursework prototype. Implement only the requested feature.

## Ownership
- Anton: src/main.c, src/ui.c, src/ui.h, src/ticket_create.c.
- Dzheffrei: src/ticket_query.c, src/ticket_edit.c, src/storage.c.
- Shared: src/app.h, src/app.c and CMakeLists.txt.
  Coordinate changes to shared types, capacities and function signatures first.
- Preserve declared contracts. Do not introduce a second Ticket/AppState or
  rename fields/functions to fit another model's generated code.
- Planned functions are declarations only. Do not call them before their
  implementation is merged. Never add stubs that claim successful work.

## Collaboration
Work in the student's own branch (anton or dzheffrei). Pull the shared main
foundation before development. Commit focused changes and merge through PRs.
Do not overwrite the other student's modules or push to their branch.
Build with cmake -S . -B build && cmake --build build before handing work over.
Check bounds, empty inputs, capacity and failure behavior for implemented features.
