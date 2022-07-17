#include "game.h"

static b32 game_started = false;

static Entity *c_player;
static Entity *c_enemy;
static Entity *clicked_icons[2];
static int clicked_icons_count = 0;
static b32 needs_helper_text = true;
static b32 is_player_turn = false;
static Entity *player_icons[4];
static Entity *enemy_icons[4];
static int last_player_icon;
static int last_enemy_icon;


void game_update()
{
	if(game_started)
	{
		if(needs_helper_text)
		{
			const int helper_text_color = 0x323232FF;
			if(clicked_icons_count == 0)
			{
				r_draw_text("Click 2 elements to combine them and perform an attack",
							-.95, .1, helper_text_color, .9f);
			}
			else if(clicked_icons_count == 1)
			{
				r_draw_text("Click 1 more element to combine them and perform an attack",
							-.95, .1, helper_text_color, .9f);
			}
		}
		
		if(clicked_icons_count == 2)
		{
#define BURN_AMOUNT 5
			if(c_player->state & E_BURNING)
			{
				c_player->hp -= BURN_AMOUNT;
			}
			if(c_enemy->state & E_BURNING)
			{
				c_enemy->hp -= BURN_AMOUNT;
			}
			
			c_player->state ^= E_IDLE;
			c_player->state |= E_CASTING;
			const char *output = perform_attack_combination(c_player, c_enemy,
															clicked_icons[0]->state,
															clicked_icons[1]->state);
			clicked_icons_count = 0;
			is_player_turn = false;
			needs_helper_text = false;
			clicked_icons[0]->color = clicked_icons[0]->default_color;
			clicked_icons[0]->is_darkened = false;
				
			clicked_icons[1]->color = clicked_icons[1]->default_color;
			clicked_icons[1]->is_darkened = false;
			
			Text_Info *text_info = ALLOC_PERM(sizeof(Text_Info));
			text_info->text = output;
			text_info->pos = V2((c_player->position.x / 10)-.3f,
								(c_player->position.y / 10)+.3f);
			text_info->color = 0x0CFFFFFF;
			text_info->scaler = 1.0f;
			
			call_func_for(4000, draw_text, text_info);
			call_func_in(4000, ai_take_turn, NULL, false);
			
			// TODO(Vasko): Casting animation and change from casting to idle
		}			
	}
	if(clicked_icons_count > 2)
	{
		V_FAIL("clicked icons out of range, resetting");
		clicked_icons_count = 0;
		clicked_icons[0]->color = clicked_icons[0]->default_color;
		clicked_icons[0]->is_darkened = false;
		
		clicked_icons[1]->color = clicked_icons[1]->default_color;
		clicked_icons[1]->is_darkened = false;
	}
	
	if(c_player && c_player->is_valid)
	{
		char hp_text[1024] = {0};
		snprintf(hp_text, 1024, "HP: %d", c_player->hp);
		r_draw_text(hp_text, c_player->position.x / 10, (c_player->position.y / 10)-.1f
					, 0xFF0A0AFF, 1.0f);
	}
	else
	{
		c_player = NULL;
	}
	if(c_enemy && c_enemy->is_valid)
	{
		char hp_text[1024] = {0};
		snprintf(hp_text, 1024, "HP: %d", c_enemy->hp);
		r_draw_text(hp_text, c_enemy->position.x / 10, (c_player->position.y / 10)-.1f, 
					0xFF0A0AFF, 1.0f);
	}
	else
	{
		c_enemy = NULL;
	}
}


void ai_take_turn(Schedule_Tracker *tracker)
{
	// TODO(Vasko): make it smart
	int index1 = rand() % 4;
	Icon_State elem1 = enemy_icons[index1]->state;
	int index2;
	while((index2 = rand() % 4) == index1) {}
	Icon_State elem2 = enemy_icons[index2]->state;
	c_player->state ^= E_IDLE;
	c_player->state |= E_CASTING;
	const char *output = perform_attack_combination(c_enemy, c_player,
													elem1,
													elem2);
	
	
	Text_Info *text_info = ALLOC_PERM(sizeof(Text_Info));
	text_info->text = output;
	text_info->pos = V2((c_enemy->position.x / 10)-.3f,
						(c_player->position.y / 10)+.3f);
	text_info->color = 0xFF0CFFFF;
	text_info->scaler = 1.0f;
	call_func_for(4000, draw_text, text_info);
	is_player_turn = true;
	// TODO(Vasko): Casting animation and change from casting to idle
}

b32 is_immune_to_element(Entity *e, Icon_State element)
{
	switch(element)
	{
		case ICON_FIRE:
		{
			if(e->state & E_FIRE_IMMUNE)
				return true;
			return false;
		} break;
		case ICON_ROCK:
		{
			if(e->state & E_ROCK_IMMUNE)
				return true;
			return false;
		} break;
		case ICON_WATER:
		{
			if(e->state & E_WATER_IMMUNE)
				return true;
			return false;
		} break;
		case ICON_WIND:
		{
			if(e->state & E_WIND_IMMUNE)
				return true;
			return false;
		} break;
		case ICON_VOID:
		{
			return false;
		} break;
		default:
		{
			V_FAIL("Wrong element passed to is_immune_to_element: %d", element);
			return false;
		}
	}
}

const char *perform_attack_combination(Entity *doer, Entity *victim,
									   Icon_State elem1, Icon_State elem2)
{
	switch(elem1)
	{
		case ICON_FIRE:
		{
			return perform_fire_combination(elem2, doer, victim);
		} break;
		case ICON_ROCK:
		{
			return perform_rock_combination(elem2, doer, victim);			
		} break;
		case ICON_WATER:
		{
			return perform_water_combination(elem2, doer, victim);
		} break;
		case ICON_WIND:
		{
			return perform_wind_combination(elem2, doer, victim);
		} break;
		case ICON_VOID:
		{
			return perform_void_combination(elem2, doer, victim);
		} break;
	}
	return "";
}

#define DEFAULT_ATTACK_DAMAGE 10

const char *perform_fire_combination(Icon_State element, Entity *doer, Entity *victim)
{
	int attack = DEFAULT_ATTACK_DAMAGE;
	if(doer->state & E_ATTACK_LOWERED)
		attack = (int)((f32)attack * .75f);
		
	switch(element)
	{
		case ICON_FIRE:
		{
			if(!is_immune_to_element(victim, ICON_FIRE))
				victim->hp -= (int)(attack * 1.5f);
			return "FIREEEEEEEEEEEEEEEEEEE";
		} break;
		case ICON_ROCK:
		{
			doer->state |= E_ROCK_CHARGED;
			return "The rock burns, next rock attack will deal double damage";
		} break;
		case ICON_WATER:
		{
			victim->state |= E_BURNING;
			return "A steam starts burning your enemy";
		} break;
		case ICON_WIND:
		{
			if(!is_immune_to_element(victim, ICON_FIRE))
				victim->hp -= attack * 3;
			return "A fire tornado forms around your enemy, ouch...";
		} break;
		case ICON_VOID:
		{
			doer->state |= E_FIRE_IMMUNE;
			return "Gained fire immunity";
		} break;
	}
	return "";
}

const char *perform_wind_combination(Icon_State element, Entity *doer, Entity *victim)
{
	int attack = DEFAULT_ATTACK_DAMAGE;
	if(doer->state & E_ATTACK_LOWERED)
		attack = (int)((f32)attack * .75f);
		
	switch(element)
	{
		case ICON_FIRE:
		{
			if(!is_immune_to_element(victim, ICON_FIRE))
				victim->hp -= attack * 3;
			return "A fire tornado forms around your enemy, ouch...";
		} break;
		case ICON_ROCK:
		{
			if(doer->state & E_ROCK_CHARGED)
			{
				doer->state ^= E_ROCK_CHARGED;
				attack *= 2;
			}
			if(!is_immune_to_element(victim, ICON_ROCK))
				victim->hp -= attack * 2;
			return "Launched rock at enemy. It did A LOT of damage";
		} break;
		case ICON_WATER:
		{
			victim->state |= E_ATTACK_LOWERED;
			return "A cloud starts raining down. Enemy attack down.";
		} break;
		case ICON_WIND:
		{
			if(!is_immune_to_element(victim, ICON_WIND))
				victim->hp -= (int)(attack * 1.5f);
			return "A giant gust of wind hits your enemy";
		} break;
		case ICON_VOID:
		{
			doer->state |= E_WIND_IMMUNE;
			return "Gained wind immunity";
		} break;
	}
	return "";
}

const char *perform_water_combination(Icon_State element, Entity *doer, Entity *victim)
{
	int attack = DEFAULT_ATTACK_DAMAGE;
	if(doer->state & E_ATTACK_LOWERED)
		attack = (int)((f32)attack * .75f);
		
	switch(element)
	{
		case ICON_FIRE:
		{
			victim->state |= E_BURNING;
			return "A steam starts burning your enemy";
		} break;
		case ICON_ROCK:
		{
			if(doer->state & E_ROCK_CHARGED)
				doer->state ^= E_ROCK_CHARGED;
			return "You sprayed a rock with water... nothing happened";
		} break;
		case ICON_WATER:
		{
			if(!is_immune_to_element(victim, ICON_WATER))
				victim->hp -= (int)(attack * 1.5f);
			return "More water more damage";
		} break;
		case ICON_WIND:
		{
			if(!is_immune_to_element(victim, ICON_WATER))
				victim->state |= E_ATTACK_LOWERED;
			return "A cloud starts raining down. Enemy attack down.";
		} break;
		case ICON_VOID:
		{
			doer->state |= E_WATER_IMMUNE;
			return "Gained water immunity";
		} break;
	}
	return "";
}

const char *perform_rock_combination(Icon_State element, Entity *doer, Entity *victim)
{
	int rock_damage = DEFAULT_ATTACK_DAMAGE;
	if(doer->state & E_ROCK_CHARGED)
	{
		doer->state ^= E_ROCK_CHARGED;
		rock_damage *= 2;
	}
	if(doer->state & E_ATTACK_LOWERED)
		rock_damage = (int)((f32)rock_damage * .75f);
		
	switch(element)
	{
		case ICON_FIRE:
		{
			doer->state |= E_ROCK_CHARGED;
			return "The rock burns, next rock attack will deal double damage";
		} break;
		case ICON_ROCK:
		{
			if(!is_immune_to_element(victim, ICON_ROCK))
				victim->hp -= (int)((f32)rock_damage * 1.5f);
			return "Launched 2 rocks at enemy. It's a bit better than 1 rock";
		} break;
		case ICON_WATER:
		{
			if(doer->state & E_ROCK_CHARGED)
				doer->state ^= E_ROCK_CHARGED;
			return "You sprayed a rock with water... nothing happened";
		} break;
		case ICON_WIND:
		{
			if(!is_immune_to_element(victim, ICON_ROCK))
				victim->hp -= rock_damage * 2;
			return "Launched rock at enemy. It did A LOT of damage";
		} break;
		case ICON_VOID:
		{
			doer->state |= E_ROCK_IMMUNE;
			return "Gained rock immunity";
		} break;
	}
	return "";
}

const char *perform_void_combination(Icon_State element, Entity *doer, Entity *victim)
{
	int attack = DEFAULT_ATTACK_DAMAGE;
	if(doer->state & E_ATTACK_LOWERED)
		attack = (int)((f32)attack * .75f);
		
	switch(element)
	{
		case ICON_FIRE:
		{
			doer->state |= E_FIRE_IMMUNE;
			return "Gained fire immunity";
		} break;
		case ICON_ROCK:
		{
			doer->state |= E_ROCK_IMMUNE;
			return "Gained rock immunity";
		} break;
		case ICON_WATER:
		{
			doer->state |= E_WATER_IMMUNE;
			return "Gained water immunity";
		} break;
		case ICON_WIND:
		{
			doer->state |= E_WIND_IMMUNE;
			return "Gained wind immunity";
		} break;
		case ICON_VOID:
		{
			victim->hp -= (int)(attack * 1.5f);
			return "Combining 2 void attacks. 150% base damage.";
		} break;
	}
	return "";
}

void animate_entity(Schedule_Tracker *tracker)
{
	Entity_Animate_Info *info = tracker->to_pass;
	if(info->current_frame >= info->frame_count)
	{
		info->current_frame = 0;
		if(info->e->state & E_CASTING)
		{
			info->e->state ^= E_CASTING;
			info->e->state |= E_IDLE;
			return;
		}
	}
	info->e->texture = get_texture_by_name(info->animation_frames[info->current_frame]).texture;
	info->current_frame++;
}


#define TIMES_TO_ROLL 5
static int ms_between_dice = 0;
static const char *icons[] = {"fire_icon", "void_icon", "rock_icon", "water_icon", "wind_icon"};
static int icons_placed = 0;

void roll_dice(Schedule_Tracker *tracker)
{
	Dice_Roll_Tracker *dice_track = tracker->to_pass;
	if(dice_track->prev_dice)
		dice_track->prev_dice->is_valid = false;
	
	dice_track->times_rolled++;
	if(dice_track->times_rolled > TIMES_TO_ROLL)
	{
		place_icon(rand() % ICON_COUNT, dice_track->player_or_enemy_flip);
		tracker->is_valid = false;
	}
	else
	{
		char dice_texture[512] = {0};
		int rolled = (rand() % 6) + 1;
		snprintf(dice_texture, 512, "die_%d", rolled);
		dice_track->prev_dice = create_entity(V4(-10, 2.5, 10, 5), dice_texture, 0xFFFFFFFF, E_NONE);
	}	
}

void place_icon(int icon_number, int flip)
{
	f32 x = flip * normalize_between(icons_placed * 2.5f, 0, 10, -8, 0);
	Entity *icon = create_entity(V4(x, 2.5, x, 4), icons[icon_number], 0xDDDDDDFF, E_CLICKABLE);
	icon->state = icon_number;
	icon->on_click = on_icon_click;
	
	icons_placed++;
	if(flip == -1)
	{
		icon->flags ^= E_CLICKABLE;
		enemy_icons[last_enemy_icon++] = icon;
	}
	else
	{
		player_icons[last_player_icon++] = icon;
	}
	
	if(icons_placed < 4)
	{
		Dice_Roll_Tracker *dice_tracker = ALLOC_PERM(sizeof(Dice_Roll_Tracker));
		memset(dice_tracker, 0, sizeof(Dice_Roll_Tracker));
		dice_tracker->player_or_enemy_flip = flip;
		
		// TODO(Vasko): change back
		//ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
		ms_between_dice = 1;
		call_func_in(ms_between_dice, roll_dice, dice_tracker, true);
		play_sound("dice");
	}
	else if(flip == 1)
		create_enemy();
	else
	{
		is_player_turn = true;
		game_started = true;
	}
}

void create_player()
{
	static const char *player_animation_frames[] = {"player_idle1", "player_idle2", "player_idle3", "player_idle4", "player_idle5", "player_idle6", "player_idle7", "player_idle8", "player_idle9", "player_idle10", "player_idle11", "player_idle12", "player_idle13", "player_idle14", "player_idle15", "player_idle16", "player_idle17"};
	
	c_player = create_entity(V4(-10, -2.5, 0, 0), "player_idle", 0xFFFFFFFF, E_CAN_DIE);
	c_player->state |= E_IDLE;
	
	Entity_Animate_Info *anim_info = ALLOC_PERM(sizeof(Entity_Animate_Info));
	memset(anim_info, 0, sizeof(Entity_Animate_Info));
	anim_info->frame_count = ARR_SIZE(player_animation_frames);
	anim_info->animation_frames = player_animation_frames;
	anim_info->e = c_player;
	
	call_func_in(100, animate_entity, anim_info, true);
	
	Dice_Roll_Tracker *dice_tracker = ALLOC_PERM(sizeof(Dice_Roll_Tracker));
	memset(dice_tracker, 0, sizeof(Dice_Roll_Tracker));
	dice_tracker->player_or_enemy_flip = 1;
	
	// TODO(Vasko): change back
	//ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
	ms_between_dice = 1;
	call_func_in(ms_between_dice, roll_dice, dice_tracker, true);
	play_sound("dice");
}

void create_enemy()
{
	static const char *enemy_animation_frames[] = {"enemy_idle"};
	
	ms_between_dice = 0;
	icons_placed = 0;
	
	c_enemy = create_entity(V4(0, -2.5, 4, 0), "enemy_idle", 0xFFFFFFFF, E_CAN_DIE);
	c_enemy->state |= E_IDLE;
	
	Entity_Animate_Info *anim_info = ALLOC_PERM(sizeof(Entity_Animate_Info));
	memset(anim_info, 0, sizeof(Entity_Animate_Info));
	anim_info->frame_count = 1;
	anim_info->animation_frames = enemy_animation_frames;
	anim_info->e = c_enemy;
	
	call_func_in(5000, animate_entity, anim_info, true);
	
	
	Dice_Roll_Tracker *dice_tracker = ALLOC_PERM(sizeof(Dice_Roll_Tracker));
	memset(dice_tracker, 0, sizeof(Dice_Roll_Tracker));
	dice_tracker->player_or_enemy_flip = -1;
	
	ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
	call_func_in(ms_between_dice, roll_dice, dice_tracker, true);
	play_sound("dice");
	
}

