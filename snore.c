#include <string.h> /* for strlen and strchr */
#include <stdlib.h> /* for fprintf putchar and fflush */
#include <stdio.h> /* for atoi */

#if !defined(SNORE_WINAPI) && defined(_MSC_VER)
#define SNORE_WINAPI
#endif

#ifdef SNORE_WINAPI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> /* for Sleep */
#include <io.h> /* for _isatty on Windows */
#else
#include <unistd.h> /* for sleep and isatty on POSIX */
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
static int isStdoutTty(void);

static int samestring(const char * a, const char * b)
{
    return 0 == strcmp(a, b);
}

static int goodnumber(const char * text)
{
    int i;

    if(strlen(text) > 9)
        return 0;

    for(i = 0; i < (int)strlen(text); ++i)
        if(!strchr("0123456789", text[i]))
            return 0;

    return atoi(text) > 0;
}

int main(int argc, char ** argv)
{
    int i, s, usecountdown;

    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s seconds [--countdown]\n", argv[0]);
        return 1;
    }

    usecountdown = 0;
    s = -1;
    for(i = 1; i < argc; ++i)
    {
        if(samestring(argv[i], "--countdown"))
        {
            usecountdown = 1;
            continue;
        }

        if(goodnumber(argv[i]))
        {
            s = atoi(argv[i]);
            continue;
        }

        fprintf(stderr, "'%s' is not valid positive number (max 9 digits) or option\n", argv[i]);
        fprintf(stderr, "Usage: %s seconds [--countdown]\n", argv[0]);
        return 2;
    }

    if(s < 0)
    {
        fprintf(stderr, "No number was given\n");
        fprintf(stderr, "Usage: %s seconds [--countdown]\n", argv[0]);
        return 3;
    }

    if(usecountdown)
    {
        const int stdouttty = isStdoutTty();
        upgradeTerminal();
        for(i = 0; i < s; ++i)
        {
            if(stdouttty)
            {
                /* move cursor to column 0, clearn line, print number, before sleep */
                printf("\r\033[K%d", s - i);
            }
            else
            {
                /* not a tty so just print number and the newline (for tools/scripts to consume) */
                printf("%d\n", s - i);
            }

            fflush(stdout); /* make sure the number is displayed */
            oneSecondSleep();
        }
    }
    else
    {
        for(i = 0; i < s; ++i)
        {
            /* 1 full minute per line */
            if(i > 0 && (i % 60) == 0) putchar('\n');
            putchar('.');

            /* in case stdout is buffered make sure the dot still appears */
            fflush(stdout);
            oneSecondSleep();
        }
    }

    if(usecountdown && isStdoutTty())
    {
        /* print a zero at the end to leave it displayed after quitting*/
        printf("\r\033[K%d", 0);
    }

    /* a newline at the end to not leave partial line and to add newline after single line countdown */
    if(!usecountdown || isStdoutTty())
        putchar('\n');

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

static int isStdoutTty(void)
{
#ifdef SNORE_WINAPI
    return _isatty(_fileno(stdout));
#else
    return isatty(1);
#endif /* SNORE_WINAPI */
}
