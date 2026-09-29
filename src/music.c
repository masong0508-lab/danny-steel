#include "music.h"
#include "data.h"

// Time base: timer 3 at F/64 = 262144 ticks/s, polled (no interrupts needed).
static u32 total, start, loop_off;
static u16 last;
static int idx;

static const u8 bass_wave[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef, 0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};

static void load_wave(void) {
    REG_SND3SEL = 0x0000;                                   // bank 0 plays -> we write bank 1
    for (int i = 0; i < 8; i++) WAVE_RAM[i] = bass_wave[2 * i] | (bass_wave[2 * i + 1] << 8);
    REG_SND3SEL = 0x0040;                                   // bank 1 plays -> we write bank 0
    for (int i = 0; i < 8; i++) WAVE_RAM[i] = bass_wave[2 * i] | (bass_wave[2 * i + 1] << 8);
    REG_SND3SEL = 0x0080;                                   // 32 samples, bank 0, on
}
static void time_update(void) { u16 n = REG_TM3CNT_L; total += (u16)(n - last); last = n; }

void music_stop(void) {
    REG_SND1CNT = 0; REG_SND2CNT = 0; REG_SND3CNT = 0; REG_SND4CNT = 0;   // volume 0 = silent
}
void music_init(void) {
    REG_SNDSTAT = 0x0080;                                   // sound on
    REG_SNDCTRL_L = 0xFF77;                                 // all 4 channels, both sides, full volume
    REG_SNDCTRL_H = 0x0002;                                 // PSG at 100%
    REG_SND1SWEEP = 0x0008;                                 // no sweep
    load_wave();
    music_stop();
    REG_TM3CNT_H = 0; REG_TM3CNT_L = 0; REG_TM3CNT_H = 0x0081;   // free-running, F/64
    last = REG_TM3CNT_L;
}
void music_start(void) {
    music_stop(); time_update();
    start = total; loop_off = 0; idx = 0;
}

static void play(const MusEv *e) {
    switch (e->ch) {
    case 0:                                                 // lead: pulse 25%, slow fade
        if (e->arg) { REG_SND1CNT = (e->arg << 12) | (6 << 8) | (1 << 6); REG_SND1FREQ = 0x8000 | e->val; }
        else REG_SND1CNT = (1 << 6);
        break;
    case 1:                                                 // comp: pulse 12.5%, held
        if (e->arg) { REG_SND2CNT = (e->arg << 12); REG_SND2FREQ = 0x8000 | e->val; }
        else REG_SND2CNT = 0;
        break;
    case 2:                                                 // bass: wave channel
        if (e->arg) { REG_SND3CNT = 0x4000; REG_SND3FREQ = 0x8000 | e->val; }
        else REG_SND3CNT = 0;
        break;
    default: {                                              // percussion: noise
        int vol = e->arg & 15;
        REG_SND4CNT = (vol << 12) | (1 << 8);               // quick decay
        REG_SND4FREQ = 0x8000 | ((e->arg >> 4) == 1 ? 0x21 : 0x5B);   // 1 = hat, 2 = side-stick
    } }
}

void music_poll(void) {
    time_update();
    u32 now = total - start;
    for (;;) {
        if (idx >= MUSIC_COUNT) { idx = MUSIC_LOOP_IDX; loop_off += MUSIC_LOOP_UNITS; }   // back to loop start
        const MusEv *e = &music_events[idx];
        if (now < e->t + loop_off) break;
        play(e); idx++;
    }
}
