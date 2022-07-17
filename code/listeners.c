
void draw_text(Schedule_Tracker *tracker)
{
	if(tracker->to_pass == NULL)
		V_FAIL("Draw text called with nothing passed");
	else
	{
		Text_Info *passed = tracker->to_pass;
		Color color = {passed->color};
		u32 passed_ms = SDL_GetTicks() - tracker->start_ms;
		f32 alpha = normalize_between((f32)passed_ms, 0, (f32)tracker->passed_ms, 1, 0);
		color.a = (u8)normalize_between(alpha, 0, 1, 0, 255);
		r_draw_text((char *)passed->text,
					passed->pos.x, passed->pos.y, color.c, passed->scaler);
	}
}

// NOTE(Vasko): Test function, not used in the final product
void test(Schedule_Tracker *tracker)
{
	V_WARN("Printed. Loop for: %d, Already passed: %d", tracker->passed_ms,
		   SDL_GetTicks() - tracker->start_ms);
}

void on_icon_click(Entity *e)
{
	if(game_started && is_player_turn)
	{
		if(clicked_icons_count == 1)
		{
			if(clicked_icons[0]->id == e->id)
				return;
		}
		darken_color(e);
		clicked_icons[clicked_icons_count++] = e;
	}
}

void on_play_click(Entity *e)
{
	e->texture = get_texture_by_name("button_pressed").texture;
}

void on_play_release(Entity *e)
{
	e->texture = get_texture_by_name("button").texture;
	clear_entities();
	room_number = 1;
	
	Entity *background = create_entity(V4(-10, -10, 10, 10),
									   "battle_background", 0xFFFFFFFF, E_NONE);
	place_entity_at_id(background, 0);
	create_player();
}
