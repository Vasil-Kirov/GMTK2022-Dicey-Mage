/* date = July 16th 2022 6:38 pm */

#ifndef GAME_H
#define GAME_H


typedef struct {
	int times_rolled;
	Entity *prev_dice;
	int player_or_enemy_flip;
} Dice_Roll_Tracker;

typedef struct {
	int frame_count;
	int current_frame;
	const char **animation_frames;
	Entity *e;
} Entity_Animate_Info;

void place_icon(int icon_number, int flip);
void create_enemy();
void on_icon_click(Entity *e);
void get_attack_combination();


typedef enum
{
	E_IDLE    = 1 << 0,
	E_CASTING = 1 << 1,
	E_BURNING = 1 << 2,
	E_FIRE_IMMUNE  = 1 << 3,
	E_ROCK_IMMUNE  = 1 << 4,
	E_WATER_IMMUNE = 1 << 5,
	E_WIND_IMMUNE  = 1 << 6,
	E_ROCK_CHARGED = 1 << 7,
	E_ATTACK_LOWERED = 1 << 8,
	E_SWITCHED_ANIMATION = 1 << 9,
} Entity_State;

typedef struct
{
	u32 color;
	f32 scaler;
	const char *text;
	v2 pos;
} Text_Info;

typedef enum
{
	ICON_FIRE,
	ICON_VOID,
	ICON_ROCK,
	ICON_WATER,
	ICON_WIND,
	ICON_COUNT
} Icon_State;

b32 ai_pick_elements(Icon_State *elem1, Icon_State *elem2);
void draw_text(Schedule_Tracker *tracker);

void ai_take_turn(Schedule_Tracker *tracker);
const char *perform_attack_combination(Entity *doer, Entity *victim,
									   Icon_State elem1, Icon_State elem2);

const char *perform_fire_combination(Icon_State element, Entity *doer, Entity *victim);
const char *perform_rock_combination(Icon_State element, Entity *doer, Entity *victim);
const char *perform_water_combination(Icon_State element, Entity *doer, Entity *victim);
const char *perform_wind_combination(Icon_State element, Entity *doer, Entity *victim);
const char *perform_void_combination(Icon_State element, Entity *doer, Entity *victim);



#endif //GAME_H
