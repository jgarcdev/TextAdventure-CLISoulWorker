#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H

#include <stddef.h>

#ifndef ARENA_ALLOCATOR_ALIGNMENT
#define ARENA_ALLOCATOR_ALIGNMENT (2*sizeof(void*))
#endif

struct ArenaAllocator {
  unsigned char* arena;
  size_t arenaSize;
  size_t prevOffset;
  size_t offset;
};

typedef unsigned char* AllocPtr;

typedef AllocPtr (*ArenaAllocatorAllocFunc)(size_t size);
typedef AllocPtr (*ArenaAllocatorReallocFunc)(AllocPtr ptr, size_t newSize);
typedef void (*ArenaAllocatorFreeFunc)(AllocPtr ptr);


/**
 * Initializes the arena allocator with the provided arena and arena size. The arena must be a valid pointer to a memory block of at 
 * least arenaSize bytes. The arena allocator will use this memory block to satisfy allocation requests until it runs out of memory.
 * @param this The arena allocator to initialize.
 * @param arena The arena to use.
 * @param arenaSize The size of the arena.
 */
void ArenaAllocatorInit(struct ArenaAllocator* this, AllocPtr arena, size_t arenaSize);
/**
 * Deinitializes the arena allocator. If a free function is provided, it will be used to free the arena. 
 * Otherwise, the arena will not be freed and it is the user's responsibility to free it if necessary.
 * @param this The arena allocator to deinitialize.
 * @param freeFunc The function to use to free the arena.
 */
void ArenaAllocatorDeinit(struct ArenaAllocator* this, ArenaAllocatorFreeFunc freeFunc);
/**
 * Clears the arena allocator. This simply resets the offsets to 0, allowing the arena to be reused without actually freeing any memory.
 * It is the user's responsibility to ensure that any pointers returned by previous allocation requests are no longer used after calling this function, as they may point to invalid memory after the offsets are reset.
 * @param this The arena allocator to clear.
 */
void ArenaAllocatorClear(struct ArenaAllocator* this);

/**
 * Allocates a block of memory of the specified size from the arena. The memory block is not initialized and may contain garbage data. 
 * The returned pointer is aligned to the default alignment defined by `ARENA_ALLOCATOR_ALIGNMENT`.
 * If the arena does not have enough memory to satisfy the allocation request, this function returns NULL.
 * @param this The arena allocator to use.
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
AllocPtr ArenaAllocatorAlloc(struct ArenaAllocator* this, size_t size);
/**
 * Allocates a block of memory of the specified size from the arena, aligned to the specified alignment. 
 * The memory block is not initialized and may contain garbage data.
 * If the arena does not have enough memory to satisfy the allocation request, this function returns NULL.
 * @param this The arena allocator to use.
 * @param size The size of the memory block to allocate.
 * @param alignment The alignment of the allocated memory block.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
AllocPtr ArenaAllocatorAlignedAlloc(struct ArenaAllocator* this, size_t size, size_t alignment);
/**
 * Reallocates a block of memory to the default alignment defined by `ARENA_ALLOCATOR_ALIGNMENT`.
 * If the pointer is to the most recently allocated block of memory, the arena allocator will attempt to resize the block in place if possible. 
 * If the pointer is not to the most recently allocated block of memory, a new block of memory will be allocated 
 * and the data from the old block will be copied to the new block. The old block will not be freed and it is the user's responsibility 
 * to ensure that any pointers to the old block are no longer used after calling this function, as they may point to invalid memory after the reallocation.
 * @param this The arena allocator to use.
 * @param ptr The pointer to the memory block to reallocate.
 * @param newSize The new size of the memory block.
 * @return A pointer to the reallocated memory block, or NULL if the reallocation fails.
 */
AllocPtr ArenaAllocatorRealloc(struct ArenaAllocator* this, AllocPtr ptr, size_t newSize);
/**
 * Reallocates a block of memory to the specified alignment. If the pointer is to the most recently allocated block of memory, 
 * the arena allocator will attempt to resize the block in place if possible. 
 * If the pointer is not to the most recently allocated block of memory, a new block of memory will be allocated and the data 
 * from the old block will be copied to the new block. The old block will not be freed and it is the user's responsibility to ensure 
 * that any pointers to the old block are no longer used after calling this function, as they may point to invalid memory after the reallocation.
 * @param this The arena allocator to use.
 * @param ptr The pointer to the memory block to reallocate.
 * @param newSize The new size of the memory block.
 * @param alignment The alignment of the reallocated memory block.
 * @return A pointer to the reallocated memory block, or NULL if the reallocation fails.
 */
AllocPtr ArenaAllocatorAlignedRealloc(struct ArenaAllocator* this, AllocPtr ptr, size_t newSize, size_t alignment);

/**
 * In the case that the arena allocator can no longer satisfy an allocation request, this function can be used to resize the arena.
 * It simply replaces the current arena with the provided one. That being, the user must provide a new arena
 * that is larger that the current one and it is the user's responsibility to copy the data from the old arena to the new one before calling this function.
 * @param this The arena allocator to resize.
 * @param	newArena The new arena to use. It must be larger than the current one and it is the user's responsibility to copy the data from the old arena to the new one before calling this function.
 * @param newArenaSize The size of the new arena. It must be larger than the current one.
 */
void ArenaAllocatorResize(struct ArenaAllocator* this, AllocPtr newArena, size_t newArenaSize);

/**
 * Same as ArenaAllocatorResize but it assumes the new arena does not have the same content as the old one.
 * That means this function copies the data from the old arena to the new one.
 * It is preferred for the user to use `ArenaAllocatorResize`.
 * @param this The arena allocator to resize.
 * @param newArena The new arena to use. It must be larger than the current one.
 * @param newArenaSize The size of the new arena. It must be larger than the current one.
 */
void ArenaAllocatorResizeMove(struct ArenaAllocator* this, AllocPtr newArena, size_t newArenaSize);

/**
 * In the case that the arena allocator can no longer satisfy an allocation request, this function can be used to resize the arena.
 * The user provides an allocation function that is used to allocate a new arena. This function must be tied to the original 
 * backing allocation of the arena allocator. For example, if the original arena allocator is backed by the heap via `malloc`, 
 * then the allocation function provided to this function must also allocate from the heap, aka providing `malloc`.
 * @param this The arena allocator to resize.
 * @param allocFunc The allocation function to use to allocate the new arena. It must be tied to the original backing allocation of the arena allocator. For example, if the original arena allocator is backed by the heap via `malloc`, then the allocation function provided to this function must also allocate from the heap, aka providing `malloc`.
 * @param newArenaSize The size of the new arena. It must be larger than the current one.
 */
void ArenaAllocatorResizeAllocate(struct ArenaAllocator* this, ArenaAllocatorAllocFunc allocFunc, size_t newArenaSize);

#endif // ARENA_ALLOCATOR_H