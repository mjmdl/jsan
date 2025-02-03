#define JSAN_IMPL
#include "jsan.h"

#define JSAN_TEST_PRINT(NAME, DATA) \
	do { \
		printf("Parsing " NAME " >>> "); \
		const char *data = DATA; \
		struct Jsan *node = jsan_parse(data, strlen(data)); \
		if (node) { \
			print_json_node(node, 0); \
			printf("\n"); \
			jsan_free(node); \
		} else { \
			printf("FAILED\n"); \
			/*return -1;*/ \
		} \
	} while (0)

char *
read_file(const char *path, size_t *out_size_opt)
{
	FILE *file = fopen(path, "rb");
	if (!file)
		return NULL;
	long length;
	char *buffer;
	if (fseek(file, 0, SEEK_END) < 0
		|| (length = ftell(file)) < 0
		|| fseek(file, 0, SEEK_SET) < 0
		|| !(buffer = malloc(length)))
	{
		fclose(file);
		return NULL;
	}
	size_t bytes_read = fread(buffer, sizeof (char), (size_t) length, file);
	fclose(file);
	if (bytes_read != length) {
		free(buffer);
		return NULL;
	}
	if (out_size_opt)
		*out_size_opt = (size_t) length;
	return buffer;
}

static void print_json_node(const struct Jsan *node, size_t depth);

static void
print_json_array(const struct Jsan *node, size_t depth)
{
	if (!node) {
		printf("---");
		return;
	}

	printf("[\n");

	for (size_t i = 0; i < node->length; i++) {
		for (size_t i = 0; i < depth; i++)
			printf("  ");

		print_json_node(&node->value.children[i], depth);
		
		if (i != node->length - 1)
			printf(",\n");
	}
	
	printf("\n");
	for (size_t i = 0; i < depth - 1; i++)
		printf("  ");
	printf("]");
}

static void
print_json_object(const struct Jsan *node, size_t depth)
{
	if (!node) {
		printf("---");
		return;
	}

	printf("{\n");

	for (size_t i = 0; i < node->length; i++) {
		const struct Jsan *child = &node->value.children[i];

		for (size_t i = 0; i < depth; i++)
			printf("  ");
		
		if (child->key)
			printf("\"%s\": ", child->key);
		
		print_json_node(child, depth);

		if (i != node->length - 1)
			printf(",\n");
	}
	
	printf("\n");
	for (size_t i = 0; i < depth - 1; i++)
		printf("  ");
	printf("}");
}

static void
print_json_node(const struct Jsan *node, size_t depth)
{
	if (!node) {
		printf("---");
		return;
	}
		
	switch (node->type) {
	case JSAN_NULL:
		printf("null");
		break;
	case JSAN_FALSE:
		printf("false");
		break;
	case JSAN_TRUE:
		printf("true");
		break;
	case JSAN_NUMBER:
		printf("%.10g", node->value.number);
		break;
	case JSAN_STRING:
		printf("\"%s\"", node->value.string);
		break;
	case JSAN_ARRAY:
		print_json_array(node, depth + 1);
		break;
	case JSAN_OBJECT:
		print_json_object(node, depth + 1);
		break;
	default:
		printf("---");
		break;
	}
}

int
main(int argc, char *argv[])
{
	if (argc >= 2) {
		size_t size;
		char *data = read_file(argv[1], &size);
		if (!data || size <= 0)
			return -1;

		struct Jsan *root = jsan_parse(data, size);
		if (!root)
			return -1;

		print_json_node(root, 0);
		printf("\n");
		
		return 0;
	}
	
	JSAN_TEST_PRINT(
		"Depth 0 null",
		"null");
	JSAN_TEST_PRINT(
		"Depth 0 false",
		"false");
	JSAN_TEST_PRINT(
		"Depth 0 true",
		"true");

	printf("\n");
	
	JSAN_TEST_PRINT(
		"Depth 0 number #0",
		"42");
	JSAN_TEST_PRINT(
		"Depth 0 number #1 (float)",
		"42.69");
	JSAN_TEST_PRINT(
		"Depth 0 number #2 (negative)",
		"-123");
	JSAN_TEST_PRINT(
		"Depth 0 number #3 (float, negative)",
		"-123.456");
	JSAN_TEST_PRINT(
		"Depth 0 number #4 (scientific notation)",
		"0.69e2");
	JSAN_TEST_PRINT(
		"Depth 0 number #5 (scientific notation, negative)",
		"-0.042e5");

	printf("\n");
	
	JSAN_TEST_PRINT(
		"Depth 0 string #0",
		"\"abcdefg\"");
	JSAN_TEST_PRINT(
		"Depth 0 string #1 (empty)",
		"\"\"");
	JSAN_TEST_PRINT(
		"Depth 0 string #2 (Lorem Ipsum)",
		"\"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\"");

	printf("\n");
	
	JSAN_TEST_PRINT(
		"Depth 0 array #0",
		"[0, 1, 2, 3]");
	JSAN_TEST_PRINT(
		"Depth 0 array #1 (empty)",
		"[]");
	JSAN_TEST_PRINT(
		"Depth 0 array #2 (invalid, missing element before coma)",
		"[, null, false]");
	JSAN_TEST_PRINT(
		"Depth 0 array #3 (invalid, missing element after coma)",
		"[true, null, ]");
	JSAN_TEST_PRINT(
		"Depth 0 array #4 (different children types)",
		"[null, false, true, -123.456, \"abcdef\"]");
	JSAN_TEST_PRINT(
		"Depth 0 array #5 (nested arrays)",
		"[[1, 2, 3], [4, 5, 6]]");
	JSAN_TEST_PRINT(
		"Depth 0 array #6 (nested arrays, various types)",
		"[null, [true, false], [[1, 2, 3], [1, 2, 3, 4]], [], null]");

	printf("\n");
	
	JSAN_TEST_PRINT(
		"Depth 0 object #0",
		"{\n"
		"\t\"first\": null,\n"
		"\t\"second\": false,\n"
		"\t\"third\": true,\n"
		"\t\"fourth\": -123.456,\n"
		"\t\"fifth\": \"abcdef\"\n"
		"}");
	JSAN_TEST_PRINT(
		"Depth 0 object #1 (nested)",
		"{\n"
		"\t\"first\": {\"a\": true, \"b\": false}\n"
		"}");

	printf("\n");

	return 0;
}
