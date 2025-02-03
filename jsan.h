#ifndef JSAN_H
#define JSAN_H

#include <stddef.h>

enum Jsan_Type {
	JSAN_NULL,
	JSAN_FALSE,
	JSAN_TRUE,
	JSAN_NUMBER,
	JSAN_STRING,
	JSAN_ARRAY,
	JSAN_OBJECT,
};

struct Jsan {
	const char *key;
	union {
		double number;
		char *string;
		struct Jsan *children;
	} value;
	size_t length;
	enum Jsan_Type type;
};

struct Jsan *jsan_parse(const char *data, size_t size);
void jsan_free(struct Jsan *root);

#endif // JSAN_H
#ifdef JSAN_IMPL

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
	}
	if (!arena->next) {
		size_t this_size = arena->end - arena->begin;
		arena->next = jsan_arena_create( (size > this_size) ? size : this_size);
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

struct Jsan_Parser {
	const char *begin;
	const char *current;
	const char *end;
	struct Jsan_Arena *arena;
	struct Jsan *node;
};

static int jsan_parser_try_value(struct Jsan_Parser *parser);

static struct Jsan_Parser
jsan_parser_make(const char *data, size_t size)
{
	struct Jsan_Parser parser = {0};
	parser.begin = data;
	parser.current = parser.begin;
	parser.end = parser.begin + size;

	size_t arena_size = sizeof (struct Jsan) * 1024;
	parser.arena = jsan_arena_create(arena_size);
	if (parser.arena)
		parser.node = jsan_arena_push(parser.arena, sizeof (struct Jsan));
	
	return parser;
}

static void
jsan_parser_trim(struct Jsan_Parser *parser)
{
	while (parser->current < parser->end
		&& isspace(*parser->current))
		parser->current++;
}

static int
jsan_parser_try_key(struct Jsan_Parser *parser)
{
	return -1;
}

static int
jsan_parser_try_number(struct Jsan_Parser *parser)
{
	char *after = NULL;
	parser->node->value.number = strtod(parser->current, &after);
	if (after == parser->current)
		return -1;
	parser->current = after;	
	return 0;
}

static int
jsan_parser_try_string(struct Jsan_Parser *parser)
{
	const char *open = parser->current++;
	while ( (parser->current < parser->end)
		&& (*parser->current != '"')
		&& (*parser->current != '\n') )
		parser->current++;
	size_t length = parser->current++ - open;

	char *string = jsan_arena_push(parser->arena, length + 1);
	if (!string)
		return -1;

	if (length > 1) {
		strncpy(string, open + 1, length - 1);
		string[length] = '\0';
	} else
		string[0] = '\0';
	parser->node->value.string = string;
	return 0;
}

static size_t
jsan_parser_count_children(const struct Jsan_Parser *parser)
{
	size_t objects = 0, arrays = 0, comas = 0, elements = 0;

	const char *cursor = parser->current;
	while (cursor < parser->end) {
		if (objects == 0 && arrays == 0) {
			if (*cursor == ']')
				break;
			else if (*cursor == ',')
				comas++;
			else if (!isspace(*cursor) && comas == elements)
				elements++;
		}
		
		if (*cursor == '{')
			objects++;
		else if (*cursor == '}')
			objects--;
		else if (*cursor == '[')
			arrays++;
		else if (*cursor == ']')
			arrays--;
		
		cursor++;
	}
	
	if (objects != 0 || arrays != 0)
		return (size_t) -1;
	if (comas > 0 && elements >= 0 && elements <= comas)
		return (size_t) -1;

	return elements;
}

static int
jsan_parser_try_array(struct Jsan_Parser *parser)
{
	parser->current++; // eat the '['
	
	struct Jsan *array = parser->node;
	array->length = jsan_parser_count_children(parser);
	if (array->length == (size_t) -1)
		return -1;
	if (array->length == 0)
		parser->current++; // eat the ']'

	if (array->length > 0) {
		size_t new_size = array->length * sizeof (struct Jsan);
		array->value.children = jsan_arena_push(parser->arena, new_size);
		if (!array->value.children)
			return -1;
	}

	for (size_t i = 0; i < array->length; i++) {
		parser->node = &array->value.children[i];

		jsan_parser_trim(parser);
		if (jsan_parser_try_value(parser) != 0)
			return -1;
		
		jsan_parser_trim(parser);
		parser->current++; // eat the ','
	}

	parser->node = array;
	return 0;
}

static int
jsan_parser_try_object(struct Jsan_Parser *parser)
{
	return -1;
}

static int
jsan_parser_try_value(struct Jsan_Parser *parser)
{
	size_t spare = parser->end - parser->current;
	switch (spare) {
	default:
	case 5:
		if (strncmp(parser->current, "false", 5) == 0) {
			parser->node->type = JSAN_FALSE;
			parser->current += 5;
			return 0;
		}
		
	case 4:
		if (strncmp(parser->current, "true", 4) == 0) {
			parser->node->type = JSAN_TRUE;
			parser->current += 4;
			return 0;
		}
		if (strncmp(parser->current, "null", 4) == 0) {
			parser->node->type = JSAN_NULL;
			parser->current += 4;
			return 0;
		}

	case 3:
	case 2:
	case 1:
		if (('0' <= *parser->current && *parser->current <= '9')
			|| '-' == *parser->current)
		{
			parser->node->type = JSAN_NUMBER;
			return jsan_parser_try_number(parser);
		}
		if (*parser->current == '"') {
			parser->node->type = JSAN_STRING;
			return jsan_parser_try_string(parser);
		}
		if (*parser->current == '[') {
			parser->node->type = JSAN_ARRAY; 
			return jsan_parser_try_array(parser);
		}
		if (*parser->current == '{') {
			parser->node->type = JSAN_OBJECT;
			return jsan_parser_try_object(parser);
		}
		
	case 0:
		return -1;
	}
}

struct Jsan *
jsan_parse(const char *data, size_t size)
{
	struct Jsan_Parser parser = jsan_parser_make(data, size);
	if (!parser.node)
		return NULL;

	struct Jsan *root = parser.node;
	if (jsan_parser_try_value(&parser) != 0) {
		jsan_arena_destroy(parser.arena);
		return NULL;
	}

	return root;
}

void
jsan_free(struct Jsan *root)
{
	free(root - 1);
}

#endif // JSAN_IMPL
