#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/*#include <assert.h>*/
#include <stringbuf.h>
#include <utf8.h>

/* We create our own version of assert() that just prints
 * the error message and continues, so that we can run all the tests even if some of them fail. */
#undef assert
#define assert_(F, L, X) do { if (!(X)) { fprintf(stderr, "%s:%d: Assertion failed: %s\n", F, L, #X); }; errs++;} while (0)
#define assert(X) assert_(__FILE__, __LINE__, X)

static int errs;

static void show_buf(stringbuf *sb)
{
	printf("[%d,%d] = %s\n", sb_len(sb), sb_chars(sb), sb_str(sb));
}

#define validate_buf(SB, EXP) validate_buf_(__FILE__, __LINE__, SB, EXP)

static void validate_buf_(const char *file, int line, stringbuf *sb, const char *expected)
{
	const char *pt = sb_str(sb);
	if (pt == NULL) {
		if (expected != NULL) {
			fprintf(stderr, "%s:%d: Error: Expected NULL, got '%s'\n", file, line, pt);
			errs++;
		}
	}
	else if (strcmp(pt, expected) != 0) {
		show_buf(sb);
		fprintf(stderr, "%s:%d: Error: Expected '%s', got '%s'\n", file, line, expected, pt);
		errs++;
	}
	sb_free(sb);
}

int main(void)
{
	stringbuf *sb;
	char *pt;

	sb = sb_alloc();
	validate_buf(sb, NULL);

	sb = sb_alloc();
	sb_append(sb, "hello");
	sb_append(sb, "world");
	validate_buf(sb, "helloworld");

	sb = sb_alloc();
	sb_append(sb, "hello");
	sb_append(sb, "world");
	sb_append(sb, "");
	sb_append(sb, "xxx");
	assert(sb_len(sb) == 13);
	validate_buf(sb, "helloworldxxx");

	sb = sb_alloc();
	sb_append(sb, "first");
	sb_append(sb, "string");
	validate_buf(sb, "firststring");

	sb = sb_alloc();
	sb_append(sb, "");
	validate_buf(sb, "");

	sb = sb_alloc();
	sb_append_len(sb, "one string here", 3);
	sb_append_len(sb, "second string here", 6);
	validate_buf(sb, "onesecond");

	sb = sb_alloc();
	sb_append_len(sb, "one string here", 3);
	sb_append_len(sb, "second string here", 6);
	pt = sb_to_string(sb);
	assert(strcmp(pt, "onesecond") == 0);
	free(pt);

	sb = sb_alloc();
	pt = sb_to_string(sb);
	assert(strcmp(pt, "") == 0);
	free(pt);

	sb = sb_alloc();
	sb_append(sb, "one");
	sb_append(sb, "three");
	sb_insert(sb, 3, "two");
	validate_buf(sb, "onetwothree");

	sb = sb_alloc();
	sb_insert(sb, 0, "two");
	sb_insert(sb, 0, "one");
	sb_insert(sb, 20, "three");
	validate_buf(sb, "onetwothree");

	sb = sb_alloc();
	sb_append(sb, "one");
	sb_append(sb, "extra");
	sb_append(sb, "two");
	sb_append(sb, "three");
	sb_delete(sb, 3, 5);
	validate_buf(sb, "onetwothree");

	sb = sb_alloc();
	sb_append(sb, "one");
	sb_append(sb, "two");
	sb_append(sb, "three");
	validate_buf(sb, "onetwothree");
	/*sb_delete(sb, 6, -1);*/
	/*validate_buf(sb, "onetwo");*/

	sb = sb_alloc();
	sb_append(sb, "one");
	sb_append(sb, "two");
	sb_append(sb, "three");
	sb_delete(sb, 0, -1);
	validate_buf(sb, "");

	sb = sb_alloc();
	sb_append(sb, "one");
	sb_append(sb, "two");
	sb_append(sb, "three");
	sb_delete(sb, 50, 20);
	validate_buf(sb, "onetwothree");

	/* OK to sb_free() a NULL pointer */
	sb_free(NULL);

#ifdef USE_UTF8
	sb = sb_alloc();
	sb_append(sb, "oneµtwo");
	assert(sb_len(sb) == 8);
	assert(sb_chars(sb) == 7);
	show_buf(sb);
	sb_delete(sb, 3, 2);
	show_buf(sb);
	assert(sb_len(sb) == 6);
	assert(sb_chars(sb) == 6);
	validate_buf(sb, "onetwo");


	/* Now test with grapheme clusters */
	sb = sb_alloc();
	sb_append(sb, "onee\u0301two"); /* 'e' + combining acute accent */
	assert(sb_len(sb) == 9);
	assert(sb_chars(sb) == 7);
	/* Delete the 'e' + combining acute accent as a single grapheme cluster */
	sb_delete(sb, 3, 3);
	assert(sb_len(sb) == 6);
	assert(sb_chars(sb) == 6);
	validate_buf(sb, "onetwo");

	/* Now test with an emoji that is a single grapheme cluster but multiple code points */
	sb = sb_alloc();
	sb_append(sb, "one" "❤️❤️" "+two");
	assert(sb_chars(sb) == 9);
	/* Delete the second heart plus the "+" */
	sb_delete_chars(sb, 4, 2);
	assert(sb_chars(sb) == 7);
	/* Now delete the first heart */
	sb_delete_chars(sb, 3, 1);
	validate_buf(sb, "onetwo");
#endif

	return(0);
}
