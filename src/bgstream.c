#include "bgstream.h"
#include "data.h"

// Resident window: 31 x 21 tiles.  Tile (tx,ty) lives in VRAM slot (ty%21)*31 + tx%31
// and in map cell (ty&31, tx&31), so the hardware's 32x32 wrap just works.
#define RES_W 31
#define RES_H 21
#define MAP ((volatile u16 *)(VRAM8 + BGS_SB * 0x800))

static int res_x, res_y;          // top-left tile of the resident window

static void load_tile(int tx, int ty) {
    int slot = (ty % RES_H) * RES_W + (tx % RES_W);
    const u32 *s = bg_tiles + (ty * IMG_TW + tx) * 16;
    volatile u32 *d = (volatile u32 *)(VRAM8 + slot * 64);
    for (int i = 0; i < 16; i++) d[i] = s[i];
    MAP[(ty & 31) * 32 + (tx & 31)] = (u16)slot;
}
static void load_col(int tx) { for (int y = res_y; y < res_y + RES_H; y++) load_tile(tx, y); }
static void load_row(int ty) { for (int x = res_x; x < res_x + RES_W; x++) load_tile(x, ty); }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void bgs_init(int sx, int sy) {
    for (int i = 0; i < 1024; i++) MAP[i] = 0;
    res_x = clampi(sx >> 3, 0, IMG_TW - RES_W);
    res_y = clampi(sy >> 3, 0, IMG_TH - RES_H);
    for (int y = res_y; y < res_y + RES_H; y++)
        for (int x = res_x; x < res_x + RES_W; x++) load_tile(x, y);
}
void bgs_update(int sx, int sy) {
    int nx = clampi(sx >> 3, 0, IMG_TW - RES_W), ny = clampi(sy >> 3, 0, IMG_TH - RES_H);
    while (res_x < nx) { load_col(res_x + RES_W); res_x++; }
    while (res_x > nx) { res_x--; load_col(res_x); }
    while (res_y < ny) { load_row(res_y + RES_H); res_y++; }
    while (res_y > ny) { res_y--; load_row(res_y); }
}
