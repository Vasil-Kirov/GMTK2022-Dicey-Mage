/* date = July 16th 2022 3:48 pm */

#ifndef SOUND_H
#define SOUND_H

typedef struct {
	u8 num_of_channels;
	u16 samples;
	int samples_per_sec;
	unsigned int bits_per_sample;
	u32 wave_len;
	u8 *wave_start;
} Sound_Info;

typedef struct {
	char *key;
	Sound_Info value;
} Audio_Table;

void play_sound(const char *sound_name);
SDL_AudioSpec load_sounds();
int get_sound_duration_in_ms(const char *sound_name);

#endif //SOUND_H
