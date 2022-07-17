#include "scheduler.h"

#define MAX_SCHEDULED 1024

static Schedule_Tracker scheduler[MAX_SCHEDULED];


void call_func_in(u32 ms, Call_Func func, void *to_pass, b32 to_repeat)
{
	FOR_EACH(scheduler)
	{
		if(!scheduler[i].is_valid)
		{
			Schedule_Tracker to_track = {
				.start_ms = SDL_GetTicks(),
				.passed_ms = ms,
				.to_repeat = to_repeat,
				.func = func,
				.to_pass = to_pass,
				.is_valid = true,
				.is_call_for = false
			};
			scheduler[i] = to_track;
			return;
		}
	}
}

void call_func_for(u32 ms, Call_Func func, void *to_pass)
{
	FOR_EACH(scheduler)
	{
		if(!scheduler[i].is_valid)
		{
			Schedule_Tracker to_track = {
				.start_ms = SDL_GetTicks(),
				.passed_ms = ms,
				.to_repeat = true,
				.func = func,
				.to_pass = to_pass,
				.is_valid = true,
				.is_call_for = true
			};
			scheduler[i] = to_track;
			return;
		}
	}
}


void check_schedules()
{
	u32 ms_now = SDL_GetTicks();
	FOR_EACH(scheduler)
	{
		if(scheduler[i].is_valid)
		{
			Schedule_Tracker *tracker = &scheduler[i];
			if(tracker->is_call_for)
			{
				if(ms_now - tracker->start_ms < tracker->passed_ms)
					tracker->func(tracker);
				else
					tracker->is_valid = false;
			}
			else
			{
				if(ms_now - tracker->start_ms >= tracker->passed_ms)
				{
					tracker->func(tracker);
					tracker->start_ms = ms_now;
					if(!tracker->to_repeat)
						tracker->is_valid = false;
				}
			}
		}
	}
}