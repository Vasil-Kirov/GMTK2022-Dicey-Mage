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
static int attack_frame = 0;
static Entity *attack_animation;


void game_update()
{
	if(game_started)
	{
		switch(room_number)
		{
			case 1:
			{
				if(!c_enemy || !c_player || !c_player->is_valid || !c_enemy->is_valid)
				{
					GAME_PANIC_OR_END:
					Text_Info *end_message = ALLOC_PERM(sizeof(Text_Info));
					end_message->color = 0xFFFFFFFF;
					end_message->scaler = 1.0f;
					end_message->pos = V2(-.9f, .8f);
					if(!c_enemy || !c_enemy->is_valid)
					{
						end_message->text = "You lost :(";
					}
					else
					{
						end_message->text = "You won :D";
					}
					call_func_for(10000, draw_text, end_message);
					
					clear_entities();
					room_number = 2;
					break;
				}
				
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
					if(!c_player || !c_player->is_valid || !c_enemy || !c_enemy->is_valid)
					{
						goto GAME_PANIC_OR_END;
					}
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
					
					c_player->state |= E_SWITCHED_ANIMATION;
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
			} break;
			case 2:
			{
				r_draw_text("Thanks for playing! <3", -.42f, 0, 0xFFFFFFFF, 2.0f);
			} break;
		}
	}
}

void ai_take_turn(Schedule_Tracker *tracker)
{
	if(!c_enemy || c_enemy->is_valid == false)
		return;
	c_enemy->state ^= E_IDLE;
	c_enemy->state |= E_CASTING;
	c_enemy->state |= E_SWITCHED_ANIMATION;
	
	Icon_State elem1, elem2;
	ai_pick_elements(&elem1, &elem2);
	const char *output = perform_attack_combination(c_enemy, c_player,
													elem1,
													elem2);
	
	
	Text_Info *text_info = ALLOC_PERM(sizeof(Text_Info));
	text_info->text = output;
	text_info->pos = V2((c_enemy->position.x / 10)-.1f,
						(c_player->position.y / 10)+.3f);
	text_info->color = 0xFF0CFFFF;
	text_info->scaler = 1.0f;
	call_func_for(5000, draw_text, text_info);
	is_player_turn = true;
}

void remove_immunities(Entity *e)
{
	if(e->state & E_FIRE_IMMUNE)
		e->state ^= E_FIRE_IMMUNE;
	if(e->state & E_WATER_IMMUNE)
		e->state ^= E_WATER_IMMUNE;
	if(e->state & E_ROCK_IMMUNE)
		e->state ^= E_ROCK_IMMUNE;
	if(e->state & E_WIND_IMMUNE)
		e->state ^= E_WIND_IMMUNE;
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
			
			remove_immunities(victim);
			victim->hp -= 5;
			return "A flood of rocks rushes through your enemy destroying\n all imunities "
				"and dealing a bit of damage";
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
			remove_immunities(victim);
			victim->hp -= 5;
			return "A flood of rocks rushes through your enemy destroying\n all imunities "
				"and dealing a bit of damage";
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

Entity_Animate_Info get_attack_animation()
{
	static const char *attack_frames[] = {"attack_animation1", "attack_animation2", 
		"attack_animation3", "attack_animation4", "attack_animation5", "attack_animation6", 
		"attack_animation7", "attack_animation8", "attack_animation9", "attack_animation10", 
		"attack_animation11", "attack_animation12", "attack_animation13", "attack_animation14", 
		"attack_animation15", "attack_animation16", "attack_animation17", "attack_animation18", 
		"attack_animation19", "attack_animation20", "attack_animation21", "attack_animation22", 
		"attack_animation23"};
	Entity_Animate_Info info;
	memset(&info, 0, sizeof(Entity_Animate_Info));
	info.current_frame = attack_frame++;
	info.frame_count = ARR_SIZE(attack_frames);
	if(attack_frame >= info.frame_count)
	{
		attack_animation->position = V4(-1000, -1000, -1000, -1000);
		attack_frame = 0;
	}
	
	info.animation_frames = attack_frames;
	info.e = attack_animation;
	return info;
}

Entity_Animate_Info get_enemy_animation()
{
	if(!c_enemy || c_enemy->is_valid == false)
		return (Entity_Animate_Info){0};
	
	
	Entity_Animate_Info anim_info;
	memset(&anim_info, 0, sizeof(Entity_Animate_Info));
	
	
	static const char *enemy_idle_animation_frames[] = {"enemy_idle1", "enemy_idle2", "enemy_idle3",
		"enemy_idle4", "enemy_idle5", "enemy_idle6", "enemy_idle7", "enemy_idle8", "enemy_idle9",
		"enemy_idle10", "enemy_idle11", "enemy_idle12", "enemy_idle13", "enemy_idle14"};
	static const char *enemy_casting_animation_frames[] = {"enemy_casting1", "enemy_casting2", 
		"enemy_casting3", "enemy_casting4", "enemy_casting5", "enemy_casting6", "enemy_casting7", 
		"enemy_casting8", "enemy_casting9", "enemy_casting10", "enemy_casting11", "enemy_casting12", 
		"enemy_casting13", "enemy_casting14", "enemy_casting15", "enemy_casting16", 
		"enemy_casting17","enemy_casting18", "enemy_casting19", "enemy_casting20", "enemy_casting21",
		"enemy_casting22","enemy_casting23", "enemy_casting24", "enemy_casting25"};
	
	static int enemy_current_frame = 0;
	
	if(c_enemy->state & E_SWITCHED_ANIMATION)
	{
		c_enemy->state ^= E_SWITCHED_ANIMATION;
		enemy_current_frame = 0;
	}
	
	
	if(c_enemy->state & E_CASTING)
	{
		anim_info.animation_frames = enemy_casting_animation_frames;
		anim_info.frame_count = ARR_SIZE(enemy_casting_animation_frames);
	}
	else if(c_enemy->state & E_IDLE)
	{
		anim_info.animation_frames = enemy_idle_animation_frames;
		anim_info.frame_count = ARR_SIZE(enemy_idle_animation_frames);
	}
	else
	{
		V_FAIL("Unkown player state %x for animation", c_enemy->state);
		return (Entity_Animate_Info){0};
	}
	
	anim_info.current_frame = enemy_current_frame++;
	anim_info.e = c_enemy;
	
	if(enemy_current_frame == 20 && c_enemy->state & E_CASTING)
	{
		if(c_player && c_player->is_valid)
		{
			attack_animation->position = c_player->position;
			attack_frame = 0;
		}
	}
	
	if(enemy_current_frame >= anim_info.frame_count)
	{
		if(c_enemy->state & E_CASTING)
		{
			c_enemy->state ^= E_CASTING;
			c_enemy->state |= E_IDLE;
		}
		enemy_current_frame = 0;
	}
	
	return anim_info;
}

Entity_Animate_Info get_player_animation()
{
	if(!c_player || c_player->is_valid == false)
		return (Entity_Animate_Info){0};
	
	Entity_Animate_Info anim_info;
	memset(&anim_info, 0, sizeof(Entity_Animate_Info));
	
	static const char *player_idle_animation_frames[] = {"player_idle1", "player_idle2", 
		"player_idle3", "player_idle4", "player_idle5", "player_idle6", "player_idle7", 
		"player_idle8", "player_idle9", "player_idle10", "player_idle11", "player_idle12", 
		"player_idle13", "player_idle14", "player_idle15", "player_idle16", "player_idle17"};
	static const char *player_casting_animation_frames[] = {"player_cast1", "player_cast2", 
		"player_cast3", "player_cast4", "player_cast5", "player_cast6", "player_cast7", 
		"player_cast8", "player_cast9", "player_cast10", "player_cast11", "player_cast12", 
		"player_cast13", "player_cast14", "player_cast15", "player_cast16", "player_cast17", 
		"player_cast18", "player_cast19", "player_cast20", "player_cast21", "player_cast22", 
		"player_cast23", "player_cast24", "player_cast25", "player_cast26", "player_cast27", \
		"player_cast28", "player_cast29", "player_cast30"};
	static int player_current_frame = 0;
	if(c_player->state & E_SWITCHED_ANIMATION)
	{
		c_player->state ^= E_SWITCHED_ANIMATION;
		player_current_frame = 0;
	}
	
	if(c_player->state & E_CASTING)
	{
		anim_info.animation_frames = player_casting_animation_frames;
		anim_info.frame_count = ARR_SIZE(player_casting_animation_frames);
	}
	else if(c_player->state & E_IDLE)
	{
		anim_info.animation_frames = player_idle_animation_frames;
		anim_info.frame_count = ARR_SIZE(player_idle_animation_frames);
	}
	else
	{
		V_FAIL("Unkown player state %x for animation", c_player->state);
		return (Entity_Animate_Info){0};
	}
	
	anim_info.current_frame = player_current_frame++;
	anim_info.e = c_player;
	
	if(player_current_frame == 20 && c_player->state & E_CASTING)
	{
		if(c_enemy && c_enemy->is_valid)
		{
			attack_animation->position = c_enemy->position;
			attack_frame = 0;
		}
	}
	
	if(player_current_frame >= anim_info.frame_count)
	{
		if(c_player->state & E_CASTING)
		{
			c_player->state ^= E_CASTING;
			c_player->state |= E_IDLE;
		}
		player_current_frame = 0;
	}
	
	return anim_info;
}

void animate_entity(Schedule_Tracker *tracker)
{
	// NOTE(Vasko): casting to a function pointer that takes no arguments
	// and returns Entity_Animate_Info
	Entity_Animate_Info info = ((Entity_Animate_Info (*)())tracker->to_pass)();
	if(info.e && info.e->is_valid)
		info.e->texture = get_texture_by_name(info.animation_frames[info.current_frame]).texture;
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
		
		ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
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
	c_player = create_entity(V4(-10, -2.5, 0, 0.2), "player_idle1", 0xFFFFFFFF, E_CAN_DIE);
	c_player->state |= E_IDLE;
	
	call_func_in(80, (Call_Func)animate_entity, (void *)get_player_animation, true);
	
	Dice_Roll_Tracker *dice_tracker = ALLOC_PERM(sizeof(Dice_Roll_Tracker));
	memset(dice_tracker, 0, sizeof(Dice_Roll_Tracker));
	dice_tracker->player_or_enemy_flip = 1;
	
	ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
	call_func_in(ms_between_dice, roll_dice, dice_tracker, true);
	play_sound("dice");
}

void create_enemy()
{
	ms_between_dice = 0;
	icons_placed = 0;
	
	c_enemy = create_entity(V4(0, -2.5, 4 , 0.5), "enemy_idle1", 0xFFFFFFFF, E_CAN_DIE);
	c_enemy->state |= E_IDLE;
	
	call_func_in(100, (Call_Func)animate_entity, (void *)get_enemy_animation, true);
	
	
	attack_animation = create_entity(V4(-1000, -1000, -1000, -1000), "attack_animation1",
									 0xFFFFFFFF, E_NONE);
	
	call_func_in(100, (Call_Func)animate_entity, (void *)get_attack_animation, true);
	
	
	Dice_Roll_Tracker *dice_tracker = ALLOC_PERM(sizeof(Dice_Roll_Tracker));
	memset(dice_tracker, 0, sizeof(Dice_Roll_Tracker));
	dice_tracker->player_or_enemy_flip = -1;
	
	ms_between_dice = get_sound_duration_in_ms("dice") / TIMES_TO_ROLL;
	call_func_in(ms_between_dice, roll_dice, dice_tracker, true);
	play_sound("dice");
	
}

inline b32 enemy_has_element(Icon_State element)
{
	for(int i = 0; i < ARR_SIZE(enemy_icons); ++i)
	{
		if(enemy_icons[i]->state == element)
			return true;
	}
	return false;
}

b32 ai_pick_elements(Icon_State *elem1, Icon_State *elem2)
{
	int random_num = rand() % 100;
	// Chceck for void usages
	if(random_num > 49 && enemy_has_element(ICON_VOID))
	{
		for(int j = 0; j < ARR_SIZE(player_icons); ++j)
		{
			Icon_State p_icon = player_icons[j]->state;
			if(p_icon == ICON_FIRE || p_icon == ICON_ROCK)
			{
				if(!is_immune_to_element(c_enemy, p_icon) &&
				   enemy_has_element(p_icon))
				{	
					*elem1 = ICON_VOID;
					*elem2 = p_icon;
					return true;
				}
			}
		}
	}
	if(c_enemy->state & E_ROCK_CHARGED)
	{
		if(enemy_has_element(ICON_ROCK))
		{
			if(enemy_has_element(ICON_WIND))
			{
				*elem1 = ICON_ROCK;
				*elem2 = ICON_WIND;
				return true;
			}
			else
			{
				int rock_counter = 0;
				for(int i = 0; i < ARR_SIZE(enemy_icons); ++i)
				{
					if(enemy_icons[i]->state == ICON_ROCK)
						rock_counter++;
				}
				if(rock_counter > 1)
				{
					*elem1 = ICON_ROCK;
					*elem2 = ICON_ROCK;
					return true;
				}
			}
		}
		else
		{
			V_FAIL("Enemy has rock charged flag but doesn't have the rock ability");
		}
	}
	
	if(random_num > 49)
	{
		if(enemy_has_element(ICON_FIRE) && enemy_has_element(ICON_ROCK) && 
		   !(c_enemy->state & E_ROCK_CHARGED))
		{
			int rock_counter = 0;
			for(int i = 0; i < ARR_SIZE(enemy_icons); ++i)
			{
				if(enemy_icons[i]->state == ICON_WIND)
				{
					*elem1 = ICON_ROCK;
					*elem2 = ICON_FIRE;
					return true;
				}
				if(enemy_icons[i]->state == ICON_ROCK)
					rock_counter++;
			}
			if(rock_counter > 1)
			{
				*elem1 = ICON_ROCK;
				*elem2 = ICON_FIRE;
				return true;
			}
		}
	}
	
	if(random_num < 50)
	{
		if(enemy_has_element(ICON_FIRE))
		{
			if(enemy_has_element(ICON_WATER) && !(c_player->state & E_BURNING))
			{
				*elem1 = ICON_FIRE;
				*elem2 = ICON_WATER;
				return true;
			}
			else if(enemy_has_element(ICON_WIND))
			{
				*elem1 = ICON_FIRE;
				*elem2 = ICON_WIND;
				return true;
			}
		}
	}
	else if(enemy_has_element(ICON_WATER) && enemy_has_element(ICON_WIND) &&
			!(c_player->state & E_ATTACK_LOWERED))
	{
		*elem1 = ICON_WATER;
		*elem2 = ICON_WIND;
		return true;
	}
	
	int index1 = rand() % 4;
	*elem1 = enemy_icons[index1]->state;
	int index2;
	while((index2 = rand() % 4) == index1) {}
	*elem2 = enemy_icons[index2]->state;
	return true;
}

