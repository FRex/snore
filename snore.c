#include <string.h> /* for strlen and strchr */
#include <stdlib.h> /* for fprintf putchar and fflush */
#include <stdio.h> /* for atoi */

#if defined(WIN32)
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

static int findunit(const char * text)
{
    if(*text == '\0')
        return 0;

    while(text[1])
        ++text;

    if(*text == 's') return 1;
    if(*text == 'm') return 60;
    if(*text == 'h') return 3600;
    return 0;
}

static int goodnumber(const char * text)
{
    size_t textlen, i;

    textlen = strlen(text);
    if(textlen == 0)
        return 0;

    if(findunit(text))
        --textlen;

    /* make sure to not attempt atoi on a string of over 9 digits */
    if(textlen > 9)
        return 0;

    for(i = 0; i < textlen; ++i)
        if(!strchr("0123456789", text[i]))
            return 0;

    return atoi(text) > 0;
}

#ifndef SNORE_VERSION
#define SNORE_VERSION 0
#endif

static void printusage(const char * argv0)
{
    if(!argv0) argv0 = "snore.exe";
    fprintf(stderr, "Usage: %s [--countdown] [--hms] time...\n", argv0);
    fprintf(stderr, "    time can include s, m or h unit (seconds, minutes, hours)\n");
    fprintf(stderr, "    --countdown uses a countdown second counter\n");
    fprintf(stderr, "    --hms uses a HH:MM:SS formatted counter\n");
    fprintf(stderr, "    --nosleep never actually sleep and print all the output at once\n");
}

/* try typedef an array of -1 elements if the given expression if false to trigger a compile error */
#define SNORE_PRIV_STATIC_ASSERT(msg, expr) typedef int SNORE_PRIV_STATIC_ASSERT_##msg[(expr) * 2 - 1]
SNORE_PRIV_STATIC_ASSERT(long_long_is_8_bytes, sizeof(long long) == 8);
SNORE_PRIV_STATIC_ASSERT(int_is_4_bytes, sizeof(int) == 4);
#undef SNORE_PRIV_STATIC_ASSERT

static void printHms(long long remaining, int stdouttty)
{
    const long long remainingseconds = remaining % 60;
    const long long remainingminutes = (remaining / 60) % 60;
    const long long remaininghours = remaining / 3600;

    if(stdouttty)
    {
        /* move cursor to column 0, clearn line, print number, before sleep */
        printf("\r\033[K%02lld:%02lld:%02lld", remaininghours, remainingminutes, remainingseconds);
    }
    else
    {
        /* not a tty so just print number and the newline (for tools/scripts to consume) */
        printf("%02lld:%02lld:%02lld\n", remaininghours, remainingminutes, remainingseconds);
    }

    fflush(stdout); /* make sure the timer is displayed */
}

static void printCountdown(long long remaining, int stdouttty)
{
    if(stdouttty)
    {
        /* move cursor to column 0, clearn line, print number, before sleep */
        printf("\r\033[K%lld", remaining);
    }
    else
    {
        /* not a tty so just print number and the newline (for tools/scripts to consume) */
        printf("%lld\n", remaining);
    }

    fflush(stdout); /* make sure the number is displayed */
}

int main(int argc, char ** argv)
{
    int usecountdown, usehms, nosleep; /* used as bools */
    long long i, s; /* 64-bit */
    int multiplier;

    if(argc < 2)
    {
        fprintf(
            stderr,
            "Snore version %d_%d_%d from https://github.com/FRex/snore\n",
            SNORE_VERSION / 10000,
            (SNORE_VERSION / 100) % 100,
            SNORE_VERSION % 100
        );
        printusage(argv[0]);
        return 1;
    }

    usecountdown = 0;
    usehms = 0;
    nosleep = 0;
    s = -1;
    for(i = 1; i < argc; ++i)
    {
        if(samestring(argv[i], "--countdown"))
        {
            usecountdown = 1;
            continue;
        }

        if(samestring(argv[i], "--hms"))
        {
            usehms = 1;
            continue;
        }

        if(samestring(argv[i], "--nosleep"))
        {
            nosleep = 1;
            continue;
        }

        if(goodnumber(argv[i]))
        {
            if(s < 0) s = 0; /* s starts at -1 so make sure its 0 before summing */
            multiplier = findunit(argv[i]);
            if(multiplier == 0) multiplier = 1; /* if there is no unit then its seconds */
            s += atoi(argv[i]) * multiplier;
            continue;
        }

        fprintf(stderr, "'%s' is not valid positive time (max 9 digits) or option\n", argv[i]);
        printusage(argv[0]);
        return 2;
    }

    if(s < 0)
    {
        fprintf(stderr, "No time was given\n");
        printusage(argv[0]);
        return 3;
    }

    if (usehms)
    {
        const int stdouttty = isStdoutTty();
        upgradeTerminal();
        for(i = 0; i < s; ++i)
        {
            printHms(s - i, stdouttty);
            if(!nosleep) oneSecondSleep();
        }

        /* print 00:00:00 at the end to leave it displayed after quitting */
        printHms(0, stdouttty);
    }
    else if(usecountdown)
    {
        const int stdouttty = isStdoutTty();
        upgradeTerminal();
        for(i = 0; i < s; ++i)
        {
            printCountdown(s - i, stdouttty);
            if(!nosleep) oneSecondSleep();
        }

        /* print a zero at the end to leave it displayed after quitting */
        printCountdown(0, stdouttty);
    }
    else
    {
        for(i = 0; i < s; ++i)
        {
            /* newline after each minute and space after each 10 seconds */
            if(i > 0 && (i % 60) == 0) putchar('\n');
            if(i > 0 && (i % 60) != 0 && (i % 10) == 0) putchar(' ');
            putchar('.');

            /* in case stdout is buffered make sure the dot still appears */
            fflush(stdout);
            if(!nosleep) oneSecondSleep();
        }
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
