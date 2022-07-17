#include "input.h"

v2 mouse_coords_to_meters(i32 x, i32 y)
{
	f32 x_pos = normalize_between((f32)x, 0, (f32)renderer.width, -1, 1); 
	f32 y_pos = -normalize_between((f32)y, 0, (f32)renderer.height,-1, 1);
	
	x_pos *= 10.0f;
	y_pos *= 10.0f;
	return V2(x_pos, y_pos);
}

void handle_input(SDL_Event event)
{
	switch(event.type)
	{
		case SDL_KEYDOWN:
		{
			
		} break;
		case SDL_MOUSEMOTION:
		{
			v2 coords = mouse_coords_to_meters(event.motion.x, event.motion.y);
			entity_mouse_collision(coords);
		} break;
		case SDL_MOUSEBUTTONDOWN:
		{
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				v2 coords = mouse_coords_to_meters(event.button.x, event.button.y);
				entity_mouse_click(coords);
			}
		} break;
		case SDL_MOUSEBUTTONUP:
		{
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				v2 coords = mouse_coords_to_meters(event.button.x, event.button.y);
				entity_mouse_release(coords);
			}
		} break;
	}
}