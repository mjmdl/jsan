#ifndef JSAN_H
#define JSAN_H

#endif // JSAN_H
#ifdef JSAN_IMPL

#include <stdlib.h>

struct Jsan_Arena {
	char *begin;
	char *current;
	char *end;
	struct Jsan_Arena *next;
};

static struct Jsan_Arena *
jsan_arena_create(size_t size)
{
	struct Jsan_Arena *arena = malloc(sizeof (struct Jsan_Arena) + size);
	if (!arena)
		return NULL;
	arena->begin = (char *) arena + sizeof (struct Jsan_Arena);
	arena->current = arena->begin;
	arena->end = arena->begin + size;
	arena->next = NULL;
	return arena;
}

static void
jsan_arena_destroy(struct Jsan_Arena *arena)
{
	if (arena->next)
		jsan_arena_destroy(arena->next);
	free(arena);
}

static void *
jsan_arena_push(struct Jsan_Arena *arena, size_t size)
{
	size_t spare = arena->end - arena->current;
	if (spare >= size) {
		char *result = arena->current;
		arena->current += size;
		return result;
	} else if (!arena->next) {
		size_t this_size = arena->end - arena->begin;
		arena->next = jsan_arena_create(size > this_size ? size : this_size);
		if (!arena->next)
			return NULL;
	}
	return jsan_arena_push(arena->next, size);
}

static void *
jsan_arena_pop(struct Jsan_Arena *arena, size_t size)
{
	if ((arena->current - size) > arena->begin)
		arena->current -= size;
	else
		arena->current = arena->begin;
	return arena->current;
}

#endif // JSAN_IMPL
