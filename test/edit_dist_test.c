
#include "../include/edit_dist.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void EXPECT(bool test, char *fmt, ...)
{
    fprintf(stdout, "Checking '");

    va_list vargs;
    va_start(vargs, fmt);
    vprintf(fmt, vargs);
    va_end(vargs);

    if (test)
        fprintf(stdout, "': (passed)\n");
    else
        fprintf(stdout, "': (failed!)\n");
}

int main(void)
{
    const char *HELLO_WORLD = "Hello World!";

    /* Null Source */
    EXPECT((edit_distn(NULL, 0, HELLO_WORLD, 12) == 12),
           "edit_distn(NULL, 0, Hello World!, 12) == 12");

    /* Null Dest */
    EXPECT((edit_distn(HELLO_WORLD, 12, NULL, 0) == 12),
           "edit_distn(Hello World!, 12, NULL, 0) == 12");

    /* Empty Source */
    EXPECT((edit_distn("", 0, HELLO_WORLD, 12) == 12),
           "edit_distn(\"\", 0, Hello World!, 12) == 12");

    /* Empty Destination */
    EXPECT((edit_distn(HELLO_WORLD, 12, "", 0) == 12),
           "edit_distn(Hello World!, 12, \"\", 0) == 12");

    /* Equal Strings */
    EXPECT((edit_distn(HELLO_WORLD, 12, HELLO_WORLD, 12) == 0),
           "edit_distn(Hello World!, 12, Hello World!, 12) == 0");

    /* Delete */
    EXPECT((edit_distn("Hello world", 11, "Hell world", 10) == 1),
           "edit_distn(Hello world, 11, Hell world, 10) == 1");

    /* Insert */
    EXPECT((edit_distn("Hell world", 10, "Hello world", 11) == 1),
           "edit_distn(Hell world, 10, Hello world, 11) == 1");

    /* Swap */
    EXPECT((edit_distn("Hello world", 11, "Hello owrld", 11) == 2),
           "edit_distn(Hello world, 11, Hello owrld, 11) == 2");

    /* Change */
    EXPECT((edit_distn("Hello world", 11, "HellX world", 11) == 2),
           "edit_distn(Hello world, 11, HellX world, 11) == 2");

    /* Test signature */
    // char *h1 = "9hDvtE7FfBli8UiWFvoxF+uY/RTnyzBzZP6QblD11Z";
    // char *h2 = "9hh76794qjH4Mn3fEO+NiWMeC+r019BrWNtlKvB";
    // char *h2 = "CKU6pnBnStIzHJnkEsB9T3K2RI3KSjlyHoTtjHapBzaQuxgaB1ij";

    // char *h1 = "ifB8PVFqH9880veTp8RQnD3WWoeW/TpZ0UQ5Wyp+6mzQ2lW3";
    // char *h2 = "TR16Lr898f8mFvX4JLUcDynuP9fJXzTgdJ0H";
    char *h1 = "TR16Lr898f8mFvX4JLUcDynuP9fJXzTgdJXb5";
    char *h2 = "TR16Lr898f8+TQa3dMJfEcE6zRyDC3lr7KkfiQSjVtDdYkOsl";

    int res = edit_distn(h1, strlen(h1), h2, strlen(h2));
    printf("%d\n", res);

    return EXIT_SUCCESS;
}
