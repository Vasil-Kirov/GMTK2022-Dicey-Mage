#include "entity.h"

Entity entities[MAX_ENTITIES] = {0};

b32 point_in_rectangle(v2 point, v4 rect)
{
	if (point.x > rect.x && point.x < rect.z && point.y > rect.y && point.y < rect.w)
        return true;
	
    return false;
}


Entity *create_entity(v4 position, const char *texture, u32 color, Entity_Flags flags)
{
	int id = -1;
	for(int i = 0; i < MAX_ENTITIES; ++i) 
	{
		if(!entities[i].is_valid)
		{
			id = i;
			break;
		}
	}
	if(id == -1)
	{
		V_FAIL("Failed to create entity, entity limit reached");
		return NULL;
	}
	Texture_Desc texture_descriptor = get_texture_by_name(texture);
	f32 entity_height = position.w - position.y;
	f32 height_in_pixels = normalize_between(entity_height, 0, 20, 0, (f32)renderer.height);
	f32 width_in_pixels = height_in_pixels * texture_descriptor.width_over_height;
	f32 entity_width = normalize_between(width_in_pixels, 0, (f32)renderer.width, 0, 20);
	f32 center = (position.x + position.z) / 2.0f;
	position.x = center - (entity_width / 2.0f);
	position.z = position.x + entity_width;
	
	Entity new_entity = {
		.id = id,
		.state = 0,
		.hp = 100,
		.position = position,
		.texture = texture_descriptor.texture,
		.color = color,
		.default_color = color,
		.is_valid = true,
		.is_glowing = false,
		.is_being_clicked = false,
		.on_hover = NULL,
		.on_click = NULL,
		.on_release = NULL,
		.on_death = NULL,
		.flags = flags
	};
	entities[id] = new_entity;
	return &entities[id];
}

void place_entity_at_id(Entity *e, int id)
{
	e->is_valid = false;
	entities[id] = *e;
	entities[id].id = id;
	entities[id].is_valid = true;
}

void clear_entities()
{
	for(int i = 0; i < MAX_ENTITIES; ++i)
	{
		entities[i].is_valid = false;
	}
}

void draw_entity(Entity e)
{
	v4 gl_pos = V4(e.position.x / 10, e.position.y / 10, e.position.z / 10, e.position.w / 10);
	r_draw_v4(gl_pos, e.texture, e.color);
}

void update_entities()
{
	for(int i = 0; i < MAX_ENTITIES; ++i)
	{
		if(entities[i].is_valid)
		{
			Entity *e = &entities[i];
			if(e->flags & E_CAN_DIE && e->hp <= 0)
			{
				if(e->on_death)
					e->on_death(e);
				e->is_valid = false;
				continue;
			}
			draw_entity(*e);
		}
	}
}

u32 truncate(u32 a)
{
	if(a > 0xFF)
		return 0xFF;
	return a;
}

void light_up_color(Entity *e)
{
	if(e->is_glowing)
		return;
	e->is_glowing = true;
	Color e_color = {e->color};
	/*
    Color reduced;
	reduced.r = 255 - e_color.r;
	reduced.b = 255 - e_color.g;
	reduced.g = 255 - e_color.b;
	
	Color to_add;
	to_add.r = (u8)(reduced.r * .5f);
	to_add.g = (u8)(reduced.g * .5f);
	to_add.b = (u8)(reduced.b * .5f);
	
	e_color.r += to_add.r;
	e_color.g += to_add.g;
	e_color.b += to_add.b;
*/
	e_color.r = truncate(e_color.r + 100);
	e_color.g = truncate(e_color.g + 100);
	e_color.b = truncate(e_color.b + 100);
	e->color = e_color.c;
}

void darken_color(Entity *e)
{
	e->is_darkened = true;
	Color e_color = {e->color};
	
	e_color.r = (u8)(e_color.r * .5f);
	e_color.g = (u8)(e_color.g * .5f);
	e_color.b = (u8)(e_color.b * .5f);
	e->color = e_color.c;
}

void entity_mouse_collision(v2 pos)
{
	for(int i = 0; i < MAX_ENTITIES; ++i)
	{
		Entity *e = &entities[i];
		if(entities[i].is_valid && e->flags & E_CLICKABLE)
		{
			if(point_in_rectangle(pos, e->position))
			{
				light_up_color(e);
				if(e->on_hover)
					e->on_hover(e);
			}
			else if(!e->is_darkened)
			{
				e->is_glowing = false;
				e->color = e->default_color;
			}
		}
	}
}

void entity_mouse_click(v2 pos)
{
	for(int i = 0; i < MAX_ENTITIES; ++i)
	{
		Entity *e = &entities[i];
		if(entities[i].is_valid && e->flags & E_CLICKABLE)
		{
			if(point_in_rectangle(pos, e->position))
			{
				if(e->on_click)
				{
					e->on_click(e);
				}
				e->is_being_clicked = true;
			}
		}
	}
}

void entity_mouse_release(v2 pos)
{
	for(int i = 0; i < MAX_ENTITIES; ++i)
	{
		Entity *e = &entities[i];
		if(entities[i].is_valid && e->is_being_clicked)
		{
				if(e->on_release)
				{
					e->on_release(e);
				}
				e->is_being_clicked = false;
		}
	}
}

