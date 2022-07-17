/* date = July 16th 2022 5:01 pm */

#ifndef MEMORY_H
#define MEMORY_H

#define PERM_SIZE MB(100)
#define TEMP_SIZE MB(100)

#define PERM_INDEX 0
#define TEMP_INDEX 1

#define ALLOC_PERM(size) allocate_memory(PERM_INDEX, size)
#define ALLOC_TEMP(size) allocate_memory(TEMP_INDEX, size)

void *
allocate_memory(int allocator_index, size_t size);

#endif //MEMORY_H
