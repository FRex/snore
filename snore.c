#include <string.h> /* for strlen and strchr */
#include <stdlib.h> /* for fprintf putchar and fflush */
#include <stdio.h> /* for atoi */

#if !defined(SNORE_WINAPI) && defined(_MSC_VER)
#define SNORE_WINAPI
#endif

#ifdef SNORE_WINAPI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> /* for Sleep */
#include <io.h> /* for _isatty */
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

static int upgradeTerminal(void);

int main(int argc, char ** argv)
{
    int i, s;

    if(argc != 2 && argc != 3)
        return fprintf(stderr, "Usage: %s seconds [--countdown]\n", argv[0]), 1;

    for(i = 0; i < (int)strlen(argv[1]); ++i)
        if(!strchr("0123456789", argv[1][i]))
            return fprintf(stderr, "Error: use only 0-9 for digits\n"), 2;

    if(strlen(argv[1]) > 9)
        return fprintf(stderr, "Error: max is 999999999 (nine 9s)\n"), 3;

    if(argc == 3 && 0 != strcmp(argv[2], "--countdown"))
        return fprintf(stderr, "Error: second argument is not '--countdown'\n"), 4;

    s = atoi(argv[1]);

    if(argc == 3)
        upgradeTerminal();

    for(i = 0; i < s; ++i)
    {
        if(argc == 3)
        {
            /* move cursor to column 0, clearn line, print number */
            printf("\r\033[K%d", s - i);
            fflush(stdout); /* make sure the number is displayed */
        }

        /* sleep after showing the countdown but before the dot */
        oneSecondSleep();

        /* print 0 after last sleep */
        if(argc == 3 && i + 1 == s)
            printf("\r\033[K%d", 0);

        if(argc == 2)
        {
            /* 1 full minute per line */
            if(i > 0 && (i % 60) == 0) putchar('\n');
            putchar('.');
        }

        /* in case stdout is line buffered make sure everything (dot or last number) apepars */
        fflush(stdout);
    } /* for i */

    putchar('\n'); /* always a newline at the end to not leave partial line */
    return 0;
}

/* this is a copy of enableConsoleColor from colors utility */
static int upgradeTerminal(void)
{
#ifndef SNORE_WINAPI
    return 1; /* outside windows just assume it will work */
#else
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0u;

    /* using 'Console' winapi func fails if stdout isn't a tty/is redirected so
     * assume we just want to dump ANSI color sequences to file in that case */
    if(!_isatty(_fileno(stdout)))
        return 1;

    if(console == INVALID_HANDLE_VALUE)
    {
        fprintf(
            stderr,
            "GetStdHandle(STD_OUTPUT_HANDLE) returned INVALID_HANDLE_VALUE, GetLastError() = %u\n",
            (unsigned)GetLastError()
        );
        return 0;
    }

    if(console == NULL)
    {
        fprintf(stderr, "GetStdHandle(STD_OUTPUT_HANDLE) returned NULL\n");
        return 0;
    }

    if(!GetConsoleMode(console, &mode))
    {
        fprintf(stderr, "GetConsoleMode(console, &mode) returned false, GetLastError() = %u\n",
            (unsigned)GetLastError());
        return 0;
    }

    /* ENABLE_VIRTUAL_TERMINAL_PROCESSING, by value in case its missing from header... */
    mode |= 0x0004;

    if(!SetConsoleMode(console, mode))
    {
        fprintf(stderr, "SetConsoleMode(console, mode) returned false, GetLastError() = %u\n",
            (unsigned)GetLastError());
        return 0;
    }

    return 1;
#endif /* SNORE_WINAPI */
}
