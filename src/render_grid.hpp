#pragma once

void render_grid_init(
  const char* grid_vsPath,
  const char* grid_fsPath,
  const int   screenW,
  const int   screenH,
  const int   gridUnit
);
void render_grid_draw();
