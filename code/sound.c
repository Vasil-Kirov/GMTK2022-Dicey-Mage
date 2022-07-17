#include "sound.h"

SDL_AudioDeviceID audio_device;
Audio_Table *sounds;

void initialize_sound()
{
	SDL_AudioSpec want, have;
	want = load_sounds();

	audio_device = SDL_OpenAudioDevice(NULL, 0,
									   &want,
									   &have,
									   0);
	if(audio_device == 0)
		V_FAIL("Couldn't get audio device");
	
}

SDL_AudioSpec load_sounds()
{
	SDL_AudioSpec result = {0};
	u8 *wave_start;
	u32 wave_len;
	SDL_LoadWAV("assets/dice.wav", &result, &wave_start, &wave_len);
	unsigned int bits_per_sample;
	if(result.format < AUDIO_U16LSB)
		bits_per_sample = 8;
	else if(result.format < AUDIO_S32LSB)
		bits_per_sample = 16;
	else if(result.format <= AUDIO_F32MSB)
		bits_per_sample = 32;
	else
	{
		V_FAIL("Unkown audio format: %x", result.format);
		bits_per_sample = 8;
	}
	
	Sound_Info sound_info = {
		.num_of_channels = result.channels,
		.samples = result.samples,
		.samples_per_sec = result.freq,
		.bits_per_sample = bits_per_sample,
		.wave_len = wave_len,
		.wave_start = wave_start,
	};
	shput(sounds, "dice", sound_info);
	return result;
}

void play_sound(const char *sound_name)
{
	Sound_Info sound = shget(sounds, sound_name);
	SDL_QueueAudio(audio_device, sound.wave_start, sound.wave_len);	
	
	SDL_PauseAudioDevice(audio_device, 0);
}

void pause_sound()
{
	SDL_PauseAudioDevice(audio_device, 1);
}

int get_sound_duration_in_ms(const char *sound_name)
{
	Sound_Info sound = shget(sounds, sound_name);
	f32 seconds = (f32)sound.wave_len /
	(f32)(sound.samples_per_sec * sound.num_of_channels * sound.bits_per_sample / 8);
	
	return (int)(seconds * 1000.0f);
}

