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
			printf("FAILED\n"); \
			/*return -1;*/ \
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
	
	printf("\n");
	
	JSAN_TEST_PRINT(
		"Depth 0 object",
		"{\n"
		"\t\"first\": null,\n"
		"\t\"second\": false,\n"
		"\t\"third\": true,\n"
		"\t\"fourth\": -123.456,\n"
		"\t\"fifth\": \"abcdef\"\n"
		"}");

	printf("\n");
	
	return 0;
}
