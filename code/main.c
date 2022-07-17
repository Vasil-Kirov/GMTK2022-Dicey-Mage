#define _CRT_SECURE_NO_WARNINGS
#include "basic.h"

#include <assert.h>

#include <glad/glad.h>
#include <glad/glad.c>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


static int room_number = 0;

#include "renderer.h"
#include "log.h"
#include "input.h"
#include "entity.h"
#include "scheduler.h"
#include "sound.h"
#include "memory.h"
#include "game.h"

#include "game.c"
#include "listeners.c"

#include "renderer.c"
#include "log.c"
#include "input.c"
#include "entity.c"
#include "scheduler.c"
#include "sound.c"
#include "memory.c"

const Init_Info resolutions[] = {
	{1600, 900},
	{1280, 720},
	{960, 540},
	{848, 480},
	{640, 360}
};

Init_Info get_display_res()
{
	SDL_DisplayMode mode;
	SDL_GetDisplayMode(0, 0, &mode);
	int mode_count = SDL_GetNumDisplayModes(0);
	if(mode_count == 0)
		mode_count = 1;
	for(int mode_i = 0; mode_i < mode_count; ++mode_i)
	{
		for(int i = 0; i < ARR_SIZE(resolutions); ++i)
		{
			if(mode.w > resolutions[i].width && mode.h > resolutions[i].height)
				return resolutions[i];
		}
	}
	V_FATAL("Couldn't query display info");
	return (Init_Info){0};
}


int main(int argc, char *argv[])
{
	srand ( (unsigned int)time(NULL) );
	
	/* Initialization */
	SDL_Init(SDL_INIT_EVERYTHING);
	atexit(SDL_Quit);
	initialize_logger();
	V_INFO("Logger initialized");
	initialize_sound();
	V_INFO("Sound system initialized");
	initialize_memroy();
	V_INFO("Memory system initialized");
	
	Init_Info info = get_display_res();
	r_init(info);
	V_INFO("Renderer initialized");
	load_sounds();
	V_INFO("Sounds loaded");
	Entity *background = create_entity(V4(-10, -10, 10, 10), "background_big", 0xFFFFFFFF, E_NONE);
	place_entity_at_id(background, 0);
	
	
	v4 bg_color = extract_color_v4_from_u32(0x0C0CACFF);
	Entity *play_button = create_entity(V4(-10, 5, 10, 7.5), "button", 0xCDCDCDFF, E_CLICKABLE);
	play_button->on_click = on_play_click;
	play_button->on_release = on_play_release;
	
	_Bool running = true;
	char fps_counter[4096] = {0};
	
	
	/* Main Loop */
	while(running)
	{
		u32 start_ticks = SDL_GetTicks();
		
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
				case SDL_QUIT: running = false; break;
				case SDL_WINDOWEVENT:
				{
					if(e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						r_resize(e.window.data1, e.window.data2);
					}
				} break;
				
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_JOYAXISMOTION:
				{
					handle_input(e);
				} break;
			}
		}
		
		update_entities();
		check_schedules();
		
		switch(room_number)
		{
			case 0: {} break;
			case 1:
			case 2:
			{
				game_update();
			} break;
		}
		
		r_draw_text(fps_counter, -.95, .8, 0xFF000CFF, 1.0f);
		r_present();
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(bg_color.x, bg_color.y, bg_color.z, bg_color.w);
		
		
		u32 end_ticks = SDL_GetTicks();
		u32 ms_this_frame = end_ticks - start_ticks;
		i32 fps = (i32)((1.0f / (f32)ms_this_frame) * 1000);

		snprintf(fps_counter, 4096, "%dms FPS: %d", end_ticks - start_ticks, fps);
	}
	clean_up_logger();
	return 0;
}
