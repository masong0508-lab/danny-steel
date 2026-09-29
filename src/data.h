#pragma once
#include "gba.h"
// ---- generated assets ----
typedef struct { u32 t; u16 val; u8 ch; u8 arg; } MusEv;      // music event (see music.c)
extern const int MUSIC_COUNT, MUSIC_LOOP_IDX;
extern const u32 MUSIC_LOOP_UNITS;
extern const MusEv music_events[];

extern const u16 bg_pal[224];
extern const u32 bg_tiles[];                                   // 64 x 56 tiles, 8bpp
#define IMG_TW 64
#define IMG_TH 56

extern const u16 text_pal[16];
extern const int text_tile_count;
extern const u32 text_tiles[];
extern const u16 title_map[], press_map[];
#define TITLE_TW 30
#define TITLE_TH 10
#define PRESS_TW 30
#define PRESS_TH 2
