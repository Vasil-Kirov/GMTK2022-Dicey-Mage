/* date = July 15th 2022 4:32 pm */

#ifndef ENTITY_H
#define ENTITY_H

#define MAX_ENTITIES 8192

struct _entity;
typedef void (*On_Func)(struct _entity *);

typedef enum
{
	E_NONE      = 0,
	E_CLICKABLE = 1 << 0,
	E_RESERVED  = 1 << 1,
	E_CAN_DIE   = 1 << 2,
} Entity_Flags;

typedef struct _entity {
	int state; // NOTE(Vasko): this is entirely for the user and is never used by the system
	unsigned int texture;
	int hp;
	u32 flags;
	u32 color;
	u32 default_color;
	b32 is_valid;
	b32 is_glowing;
	b32 is_being_clicked;
	b32 is_darkened;
	size_t id;
	On_Func on_hover;
	On_Func on_click;
	On_Func on_release;
	On_Func on_death;
	v4 position;
} Entity;


Entity *create_entity(v4 position, const char *texture, u32 color, Entity_Flags flags);
void entity_mouse_collision(v2 pos);
void entity_mouse_click(v2 pos);
void entity_mouse_release(v2 pos);
void clear_entities();
void place_entity_at_id(Entity *e, int id);
void darken_color(Entity *e);

#endif //ENTITY_H
