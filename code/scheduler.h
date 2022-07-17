/* date = July 16th 2022 2:28 pm */

#ifndef SCHEDULER_H
#define SCHEDULER_H

struct _tracker;
typedef void (*Call_Func)(struct _tracker *);

typedef struct _tracker {
	u32 start_ms;
	u32 passed_ms;
	b32 to_repeat;
	Call_Func func;
	void *to_pass;
	b32 is_valid;
	b32 is_call_for;
} Schedule_Tracker;

void call_func_in(u32 ms, Call_Func func, void *to_pass, b32 to_repeat);
void call_func_for(u32 ms, Call_Func func, void *to_pass);

#endif //SCHEDULER_H
