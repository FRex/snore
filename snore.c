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

static int g_isStdoutTty;

static int isStdoutTty(void)
{
    if(g_isStdoutTty < 0)
    {
#ifdef SNORE_WINAPI
    g_isStdoutTty = !!_isatty(_fileno(stdout));
#else
    g_isStdoutTty = !!isatty(1);
#endif /* SNORE_WINAPI */
    }

    return g_isStdoutTty;
}

static int upgradeTerminal(void);

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
    fprintf(stderr, "    --help print this help\n");
    fprintf(stderr, "    --countdown uses a countdown second counter\n");
    fprintf(stderr, "    --hms uses a HH:MM:SS formatted counter\n");
    fprintf(stderr, "    --nosleep never actually sleep and print all the output at once\n");
    fprintf(stderr, "    --sum only sum the times and print the total seconds\n");
    fprintf(stderr, "    --print print times as they are being parsed\n");
    fprintf(stderr, "    --timer create a timer counting up in HH:MM:SS format\n");
}

/* try typedef an array of -1 elements if the given expression if false to trigger a compile error */
#define SNORE_PRIV_STATIC_ASSERT(msg, expr) typedef int SNORE_PRIV_STATIC_ASSERT_##msg[(expr) * 2 - 1]
SNORE_PRIV_STATIC_ASSERT(long_long_is_8_bytes, sizeof(long long) == 8);
SNORE_PRIV_STATIC_ASSERT(int_is_4_bytes, sizeof(int) == 4);
#undef SNORE_PRIV_STATIC_ASSERT

static void printHms(long long remaining)
{
    const long long remainingseconds = remaining % 60;
    const long long remainingminutes = (remaining / 60) % 60;
    const long long remaininghours = remaining / 3600;
    char buff[256]; /* NOTE: sprintf makes cursor not jump around in cmd.exe so its beneficial */

    if(isStdoutTty())
    {
        /* move cursor to column 0, clearn line, print number, before sleep */
        sprintf(buff, "\r\033[K%02lld:%02lld:%02lld", remaininghours, remainingminutes, remainingseconds);
    }
    else
    {
        /* not a tty so just print number and the newline (for tools/scripts to consume) */
        sprintf(buff, "%02lld:%02lld:%02lld\n", remaininghours, remainingminutes, remainingseconds);
    }

    fputs(buff, stdout);
    fflush(stdout); /* make sure the timer is displayed */
}

static void printCountdown(long long remaining)
{
    char buff[256]; /* NOTE: sprintf makes cursor not jump around in cmd.exe so its beneficial */

    if(isStdoutTty())
    {
        /* move cursor to column 0, clearn line, print number, before sleep */
        sprintf(buff, "\r\033[K%lld", remaining);
    }
    else
    {
        /* not a tty so just print number and the newline (for tools/scripts to consume) */
        sprintf(buff, "%lld\n", remaining);
    }

    fputs(buff, stdout);
    fflush(stdout); /* make sure the number is displayed */
}

static void printDots(long long i)
{
    /* newline after each minute and space after each 10 seconds */
    if(i > 0 && (i % 60) == 0) putchar('\n');
    if(i > 0 && (i % 60) != 0 && (i % 10) == 0) putchar(' ');
    putchar('.');

    /* in case stdout is buffered make sure the dot still appears */
    fflush(stdout);
}

static int hasoption(int argc, char ** argv, const char * opt)
{
    int i;
    for(i = 1; i < argc; ++i)
        if(samestring(argv[i], opt))
            return 1;
    return 0;
}

static int timer(void)
{
    long long sofar;
    upgradeTerminal();
    sofar = 0;
    while(1)
    {
        printHms(sofar++);
        oneSecondSleep();
    } // while 1
    return 2;
}

int main(int argc, char ** argv)
{
    int printtimes, onlysum, usecountdown, usehms, nosleep; /* used as bools */
    long long i, s, multiplier; /* 64-bit, to use as times */

    g_isStdoutTty = -1; /* to make first isStdoutTty call update the cache */

    if(hasoption(argc, argv, "--timer"))
    {
        if(argc != 2)
        {
            fprintf(stderr, "if --timer option is present it must be the only option\n");
            return 1;
        }

        return timer();
    }

    /* short posix and long gnu and two common ms dos help options so 4 in total */
    if(argc < 2 || hasoption(argc, argv, "--help") || hasoption(argc, argv, "-h") || hasoption(argc, argv, "/?") || hasoption(argc, argv, "-?"))
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
    onlysum = 0;
    printtimes = hasoption(argc, argv, "--print");
    s = -1;

    for(i = 1; i < argc; ++i)
    {
        if(samestring(argv[i], "--print"))
            continue;

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

        if(samestring(argv[i], "--sum"))
        {
            onlysum = 1;
            continue;
        }

        if(goodnumber(argv[i]))
        {
            long long toadd;

            if(s < 0) s = 0; /* s starts at -1 so make sure its 0 before summing */
            multiplier = findunit(argv[i]);
            if(multiplier == 0) multiplier = 1; /* if there is no unit then its seconds */
            toadd = atoi(argv[i]) * multiplier;
            s += toadd;
            if(printtimes)
                fprintf(stderr, "'%s' parsed as %lld seconds\n", argv[i], toadd);
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

    if(onlysum)
    {
        printf("The total is %lld seconds\n", s);
        return 0;
    }

    if (usehms)
    {
        upgradeTerminal();
        for(i = 0; i < s; ++i)
        {
            printHms(s - i);
            if(!nosleep) oneSecondSleep();
        }

        /* print 00:00:00 at the end to leave it displayed after quitting */
        printHms(0);
    }
    else if(usecountdown)
    {
        upgradeTerminal();
        for(i = 0; i < s; ++i)
        {
            printCountdown(s - i);
            if(!nosleep) oneSecondSleep();
        }

        /* print a zero at the end to leave it displayed after quitting */
        printCountdown(0);
    }
    else
    {
        for(i = 0; i < s; ++i)
        {
            printDots(i);
            if(!nosleep) oneSecondSleep();
        }
    }

    /* if its not hms or countdown (so its dots) then put newline at the end and if its
       printing to tty as single line hms or countdown then put newline at the end too */
    if((!usecountdown && !usehms) || isStdoutTty())
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
