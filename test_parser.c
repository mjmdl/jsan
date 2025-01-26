#define JSAN_IMPL
#include "jsan.h"

#define JSAN_TEST_PRINT(NAME, DATA) \
	do { \
		printf("Parsing " NAME " >>> "); \
		const char *data = DATA; \
		struct Jsan *node = jsan_parse(data, strlen(data) ); \
		if (node) { \
			jsan_print(node); \
			printf("\n"); \
			jsan_free(node); \
		} else { \
			printf("Failed to parse " NAME "\n"); \
			return -1; \
		} \
	} while (0)

int
main(void)
{
	JSAN_TEST_PRINT(
		"Depth 0 null",
		"null");
	JSAN_TEST_PRINT(
		"Depth 0 false",
		"false");
	JSAN_TEST_PRINT(
		"Depth 0 true",
		"true");
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
	JSAN_TEST_PRINT(
		"Depth 0 string #0",
		"\"abcdefg\"");
	JSAN_TEST_PRINT(
		"Depth 0 string #1 (empty)",
		"\"\"");
	JSAN_TEST_PRINT(
		"Depth 0 array #0",
		"\[0, 1, 2, 3]");
	JSAN_TEST_PRINT(
		"Depth 0 array #1 (empty)",
		"\[]");
	JSAN_TEST_PRINT(
		"Depth 0 array #2 (different children types)",
		"\[null, false, true, -123.456, \"abcdef\"]");
	JSAN_TEST_PRINT(
		"Depth 0 object",
		"{\n"
		"\t\"first\": null,\n"
		"\t\"second\": false,\n"
		"\t\"third\": true,\n"
		"\t\"fourth\": -123.456,\n"
		"\t\"fifth\": \"abcdef\"\n"
		"}");

	return 0;
}
