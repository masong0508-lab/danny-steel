// THE LAST DAYS of danny steel - title screen (placeholder logic)
//   * drifting, digitised painting as background (streamed from ROM)
//   * grunge title + blinking PRESS START
//   * START -> fade out -> the title screen restarts (music too). Replace title_finished() later.
#include "gba.h"
#include "data.h"
#include "bgstream.h"
#include "music.h"
#include "logo.h"

#define TITLE_ROW 1                       // map rows for the text layers
#define PRESS_ROW 16

enum { ST_FADE_IN, ST_RUN, ST_FADE_OUT };
static int state, state_t, frames, was_down;
static u32 ph_x, ph_y;                    // scroll phases, 16.16 turns
static s16 sin_lut[256];                  // Q12

static void build_sin(void) {             // integer sine (Bhaskara approximation), no libm
    for (int a = 0; a < 256; a++) {
        int h = a & 127, p = h * (128 - h), v = 65536 * p / (81920 - 4 * p);
        sin_lut[a] = (s16)(a >= 128 ? -v : v);
    }
}
static int sin12(u32 ph) {
    int i = (ph >> 8) & 255, f = ph & 255, a = sin_lut[i], b = sin_lut[(i + 1) & 255];
    return a + (((b - a) * f) >> 8);
}
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
static void scroll_pos(int *sx, int *sy) {
    *sx = clampi(BGS_MAX_X / 2 + ((BGS_MAX_X / 2 * sin12(ph_x)) >> 12), 0, BGS_MAX_X);
    *sy = clampi(BGS_MAX_Y / 2 + ((BGS_MAX_Y / 2 * sin12(ph_y)) >> 12), 0, BGS_MAX_Y);
}

// Build the whole title screen from scratch (used at boot and every time we "loop back").
static void title_setup(void) {
    REG_DISPCNT = DCNT_BLANK; REG_BLDY = 16;
    music_stop();
    for (int i = 0; i < 224; i++) PAL16[i] = bg_pal[i];
    for (int i = 0; i < 16; i++) PAL16[224 + i] = text_pal[i];            // palette bank 14 = text
    volatile u32 *tt = (volatile u32 *)(VRAM8 + 3 * 0x4000);              // charblock 3 = text tiles
    for (int i = 0; i < text_tile_count * 8; i++) tt[i] = text_tiles[i];
    volatile u16 *m0 = (volatile u16 *)(VRAM8 + 30 * 0x800), *m1 = (volatile u16 *)(VRAM8 + 31 * 0x800);
    for (int i = 0; i < 1024; i++) { m0[i] = 0; m1[i] = 0; }
    for (int r = 0; r < TITLE_TH; r++) for (int c = 0; c < TITLE_TW; c++) m0[(TITLE_ROW + r) * 32 + c] = title_map[r * TITLE_TW + c];
    for (int r = 0; r < PRESS_TH; r++) for (int c = 0; c < PRESS_TW; c++) m1[(PRESS_ROW + r) * 32 + c] = press_map[r * PRESS_TW + c];
    ph_x = ph_y = 0; frames = 0;
    int sx, sy; scroll_pos(&sx, &sy);
    bgs_init(sx, sy);
    REG_BGCNT(0) = BGCNT(0, 3, 30);                                       // title text
    REG_BGCNT(1) = BGCNT(0, 3, 31);                                       // press start
    REG_BGCNT(2) = BGCNT(3, 0, BGS_SB) | BGCNT_8BPP;                      // painting
    REG_BGHOFS(0) = REG_BGVOFS(0) = REG_BGHOFS(1) = REG_BGVOFS(1) = 0;
    REG_BGHOFS(2) = sx & 255; REG_BGVOFS(2) = sy & 255;
    REG_BLDCNT = 0x00FF;                                                  // fade-to-black on everything
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2;
    music_start();
    state = ST_FADE_IN; state_t = 0;
}

// PLACEHOLDER game logic: for now START simply sends us back to the title screen.
// Later: start the game / show a menu here instead.
static void title_finished(void) { title_setup(); }

static void frame_update(void) {
    frames++;
    ph_x += 22; ph_y += 33;                                               // ~50 s and ~33 s per loop
    int sx, sy; scroll_pos(&sx, &sy);
    bgs_update(sx, sy);
    REG_BGHOFS(2) = sx & 255; REG_BGVOFS(2) = sy & 255;

    int down = !(REG_KEYINPUT & KEY_START);
    int pressed = down && !was_down; was_down = down;
    int blink_on;
    switch (state) {
    case ST_FADE_IN:
        REG_BLDY = clampi(16 - state_t * 16 / 24, 0, 16); blink_on = 1;
        if (++state_t >= 24) { state = ST_RUN; state_t = 0; }
        break;
    case ST_RUN:
        REG_BLDY = 0; blink_on = ((frames / 30) & 1) == 0;
        if (pressed) { state = ST_FADE_OUT; state_t = 0; }
        break;
    default:                                                              // ST_FADE_OUT
        REG_BLDY = clampi(state_t * 16 / 24, 0, 16); blink_on = (state_t >> 2) & 1;   // fast blink
        if (++state_t >= 24) { title_finished(); return; }
    }
    REG_DISPCNT = DCNT_BG0 | DCNT_BG2 | (blink_on ? DCNT_BG1 : 0);
}

int main(void) {
    logo_play();                                                          // boot logo (~8 s), then straight into the title screen
    build_sin();
    music_init();
    title_setup();
    int in_vblank = 0;
    for (;;) {
        music_poll();                                                     // keeps the song on time
        if (REG_VCOUNT >= 160) { if (!in_vblank) { in_vblank = 1; frame_update(); } }
        else in_vblank = 0;
    }
}
