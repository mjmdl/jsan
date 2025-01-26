#define JSAN_IMPL
#include "jsan.h"

#include <stdio.h>

static void
print_arena_ints(struct Jsan_Arena *arena, size_t index)
{
	printf("Arena #%zu\n", index);
	
	for (int *num = (int *) arena->begin; num < (int *) arena->current; num++)
		printf("#%d\n", *num);

	if (arena->next)
		print_arena_ints(arena->next, index + 1);
}

int
main(void)
{
	struct Jsan_Arena *arena = jsan_arena_create(sizeof (int) * 128);
	for (int i = 0; i < 500; i++) {
		int *ptr = jsan_arena_push(arena, sizeof (int));
		if (!ptr)
			break;
		*ptr = i;
	}

	print_arena_ints(arena, 0);
	
	jsan_arena_destroy(arena);
	return 0;
}
