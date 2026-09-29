#pragma once
// Plays the song on the GBA's 4 built-in sound channels (2 pulse, 1 wave, 1 noise).
// music_start(): restart from the 4-bar intro.  music_poll(): call as often as you can (>100x/second).
void music_init(void);
void music_start(void);
void music_stop(void);
void music_poll(void);
