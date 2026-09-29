#pragma once
// Minimal GBA hardware definitions (no libgba / libtonc needed).
#include <stdint.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int16_t s16; typedef int32_t s32;

#ifdef HOST_TEST                        // lets tools/test_bgstream.c run the streaming code on a PC
extern u8 host_vram[0x18000];
extern u16 host_pal[512];
#define VRAM8 (host_vram)
#define PAL16 (host_pal)
#else
#define VRAM8 ((volatile u8 *)0x06000000)
#define PAL16 ((volatile u16 *)0x05000000)
#endif

#define R16(a) (*(volatile u16 *)(a))
#define REG_DISPCNT   R16(0x04000000)
#define REG_VCOUNT    R16(0x04000006)
#define REG_BGCNT(n)  R16(0x04000008 + (n) * 2)
#define REG_BGHOFS(n) R16(0x04000010 + (n) * 4)
#define REG_BGVOFS(n) R16(0x04000012 + (n) * 4)
#define REG_BLDCNT    R16(0x04000050)
#define REG_BLDY      R16(0x04000054)
#define REG_KEYINPUT  R16(0x04000130)
#define KEY_START 8
#define KEY_A 1

#define DCNT_BG0 0x0100
#define DCNT_BG1 0x0200
#define DCNT_BG2 0x0400
#define DCNT_BLANK 0x0080
#define BGCNT(prio, cb, sb) ((prio) | ((cb) << 2) | ((sb) << 8))   // 32x32 map, 4bpp
#define BGCNT_8BPP 0x0080

// sound (PSG channels)
#define REG_SND1SWEEP R16(0x04000060)
#define REG_SND1CNT   R16(0x04000062)
#define REG_SND1FREQ  R16(0x04000064)
#define REG_SND2CNT   R16(0x04000068)
#define REG_SND2FREQ  R16(0x0400006C)
#define REG_SND3SEL   R16(0x04000070)
#define REG_SND3CNT   R16(0x04000072)
#define REG_SND3FREQ  R16(0x04000074)
#define REG_SND4CNT   R16(0x04000078)
#define REG_SND4FREQ  R16(0x0400007C)
#define REG_SNDCTRL_L R16(0x04000080)
#define REG_SNDCTRL_H R16(0x04000082)
#define REG_SNDSTAT   R16(0x04000084)
#define WAVE_RAM ((volatile u16 *)0x04000090)
#define REG_TM3CNT_L  R16(0x0400010C)
#define REG_TM3CNT_H  R16(0x0400010E)
