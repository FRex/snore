#include <Windows.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    int i, s;

    if(argc != 2)
        return fprintf(stderr, "Usage: %s seconds\n", argv[0]), 1;

    for(i = 0; i < (int)strlen(argv[1]); ++i)
        if(NULL == strchr("0123456789", argv[1][i]))
            return fprintf(stderr, "Error: use only 0-9 for digits\n", argv[0]), 2;

    if(strlen(argv[1]) > 9)
        return fprintf(stderr, "Error: max is 999999999 (nine 9s)\n", argv[0]), 3;

    s = atoi(argv[1]);
    for(i = 0; i < s; ++i)
    {
        Sleep(1000u);
        if(i > 0 && (i % 60) == 0) putchar('\n'); /* 1 full minute per line */
        putchar('.');
        fflush(stdout);
    }

    putchar('\n');
    return 0;
}
