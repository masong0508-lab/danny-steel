#pragma once
// Scrolls a 512x448 8bpp picture through a 256x256 hardware background by
// streaming tile rows/columns into VRAM as the view moves.
#define BGS_SB 22                 // screenblock used for the map
#define BGS_MAX_X (512 - 240)
#define BGS_MAX_Y (448 - 160)
void bgs_init(int sx, int sy);    // load everything for this view (call in forced blank)
void bgs_update(int sx, int sy);  // call every frame before writing the scroll registers
