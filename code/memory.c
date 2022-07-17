#include "memory.h"


static void *memory_allocators[2];
static void *mem_start_locations[2];

void
initialize_memroy()
{
	memory_allocators[PERM_INDEX] = SDL_malloc(PERM_SIZE);
	memory_allocators[TEMP_INDEX] = SDL_malloc(TEMP_SIZE);
	mem_start_locations[PERM_INDEX] = memory_allocators[PERM_INDEX];
	mem_start_locations[TEMP_INDEX] = memory_allocators[TEMP_INDEX];
}

void *
allocate_memory(int allocator_index, size_t size)
{
	void *result = memory_allocators[allocator_index];
	memory_allocators[allocator_index] = (u8 *)memory_allocators[allocator_index] + size;
	if((u8 *)memory_allocators[PERM_INDEX] - (u8 *)mem_start_locations[PERM_INDEX] > PERM_SIZE)
	{
		V_FATAL("permanent memory overflow");
	}
	if((u8 *)memory_allocators[TEMP_INDEX] - (u8 *)mem_start_locations[TEMP_INDEX] > TEMP_SIZE)
	{
		V_FATAL("temporary memory overflow");
	}
	   
	return result;
}