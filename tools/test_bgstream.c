// Host-side test of the tile streaming: build with  gcc -DHOST_TEST -Isrc tools/test_bgstream.c src/bgstream.c src/bg_data.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/data.h"
#include "../src/bgstream.h"
u8 host_vram[0x18000]; u16 host_pal[512];

static int check(int sx, int sy) {                    // what would the GBA draw? compare with the source picture
    u16 *map = (u16 *)(host_vram + BGS_SB * 0x800);
    for (int Y = 0; Y < 160; Y += 1) for (int X = 0; X < 240; X += 1) {
        int px = sx + X, py = sy + Y;                 // image pixel that should appear here
        int tx = px >> 3, ty = py >> 3;               // hardware: map cell from (scroll + screen) mod 256
        int hx = ((sx & 255) + X) >> 3 & 31, hy = ((sy & 255) + Y) >> 3 & 31;
        int slot = map[hy * 32 + hx];
        const u8 *got = host_vram + slot * 64 + ((py & 7) * 8 + (px & 7));
        const u8 *want = (const u8 *)bg_tiles + (ty * IMG_TW + tx) * 64 + ((py & 7) * 8 + (px & 7));
        if (*got != *want) { printf("MISMATCH at scroll(%d,%d) screen(%d,%d)\n", sx, sy, X, Y); return 0; }
    }
    return 1;
}
int main(void) {
    srand(1); int bad = 0, n = 0;
    int sx = 100, sy = 100; bgs_init(sx, sy); bad += !check(sx, sy); n++;
    for (int i = 0; i < 4000; i++) {                  // slow drifts in all directions
        sx += rand() % 5 - 2; sy += rand() % 5 - 2;
        if (sx < 0) sx = 0; if (sx > BGS_MAX_X) sx = BGS_MAX_X; if (sy < 0) sy = 0; if (sy > BGS_MAX_Y) sy = BGS_MAX_Y;
        bgs_update(sx, sy); if (i % 7 == 0) { bad += !check(sx, sy); n++; }
    }
    for (int i = 0; i < 300; i++) {                   // big random jumps + the four corners
        sx = rand() % (BGS_MAX_X + 1); sy = rand() % (BGS_MAX_Y + 1);
        if (i < 4) { sx = (i & 1) ? BGS_MAX_X : 0; sy = (i & 2) ? BGS_MAX_Y : 0; }
        bgs_update(sx, sy); bad += !check(sx, sy); n++;
    }
    printf("%d views checked, %d bad\n", n, bad);
    return bad != 0;
}
