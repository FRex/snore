#include <string.h> /* for strlen and strchr */
#include <stdlib.h> /* for fprintf putchar and fflush */
#include <stdio.h> /* for atoi */

#if !defined(SNORE_WINAPI) && defined(_MSC_VER)
#define SNORE_WINAPI
#endif

#ifdef SNORE_WINAPI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> /* for Sleep */
#else
#include <unistd.h> /* for sleep */
#endif

static void oneSecondSleep(void)
{
#ifdef SNORE_WINAPI
    Sleep(1000u);
#else
    sleep(1u);
#endif
}

int main(int argc, char ** argv)
{
    int i, s;

    if(argc != 2)
        return fprintf(stderr, "Usage: %s seconds\n", argv[0]), 1;

    for(i = 0; i < (int)strlen(argv[1]); ++i)
        if(!strchr("0123456789", argv[1][i]))
            return fprintf(stderr, "Error: use only 0-9 for digits\n"), 2;

    if(strlen(argv[1]) > 9)
        return fprintf(stderr, "Error: max is 999999999 (nine 9s)\n"), 3;

    s = atoi(argv[1]);
    for(i = 0; i < s; ++i)
    {
        oneSecondSleep();
        if(i > 0 && (i % 60) == 0) putchar('\n'); /* 1 full minute per line */
        putchar('.');
        fflush(stdout); /* flush the dot out in case output is line buffered */
    }

    putchar('\n'); /* always a newline at the end to not leave partial line */
    return 0;
}
