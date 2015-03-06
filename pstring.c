#define PSTRING_C_TEST
#include "pstring.h"
#include <stdio.h>
#include <sys/time.h>

static struct timeval g_timer;
static void reset_timer()
{
    gettimeofday(&g_timer, NULL);
}

static void show_timer(const char *s)
{
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    double sec = (endtime.tv_sec - g_timer.tv_sec)
        + (double)(endtime.tv_usec - g_timer.tv_usec) / 1000 / 1000;
    printf("%20s: %f sec\n", s, sec);
}

static int bench(int (*func)(const char *, const char *, unsigned))
{
    const struct {
        const char text[40];
        const char key[40];
        int expect;
    } tbl[] = {
        { "abcdefgxxx", "abcdefg", 1 },
        { "abcdefgxxx", "abcdefg", 1 },
        { "bbcDefGxxx", "abcdefg", 0 },
        { "01234567890abcdpqrsefg", "01234567890abcdpqrs", 1 },
        { "a0s9vnfa3wvfa38v4ran$#svdffsfsdff", "a0s9vnfa3wvfa38v4ran$#svdffsfsdf", 1 },
        { "a0s9vnfa3wvfa38V4ran$#svdffsfxdff", "a0s9vnfa3wvfa38v4ran$#svdffsfsdf", 0 },
        { "bxxxxx", "a", 0 },
        { "bxxxxx", "b", 1 },
    };

    unsigned i, j, z;
    int err = 0;
    for (z = 0; z < 4; z++) {
        reset_timer();
        for (j = 0; j < 8*1024*1024; j++) {
            for (i = 0; i < sizeof(tbl) / sizeof(*tbl); i++) {
                const char *text = tbl[i].text;
                const char *key = tbl[i].key;
                int ret = func(text, key, strlen(key));
                if (ret != tbl[i].expect) {
                    printf("ERR %d %d %d\n", i, ret, tbl[i].expect);
                    err++;
                }
            }
        }
        show_timer("");
    }
    return err;
}

int main(int argc, char const* argv[])
{
    int err = 0;
#ifdef __AVX2__
    puts("use AVX2");
    err  = bench(pstring_starts_with);
#endif
    puts("use simple");
    err += bench(pstring_starts_with_simple);
    puts("use strcmp");
    err += bench(pstring_starts_with_strcmp);
    if (err == 0) {
        puts("test ok");
    }
    return 0;
}
