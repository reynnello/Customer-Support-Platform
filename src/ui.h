#ifndef SUPPORT_UI_H
#define SUPPORT_UI_H
#include "app.h"
/* Called once per frame before drawing. Owns input handling. */
void ui_update(AppState *app);
/* Called between BeginDrawing and EndDrawing. Does not mutate app state. */
void ui_draw(const AppState *app);
#endif
