#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <x86intrin.h>
#define PSTRING_C_TEST

#define OFFSET_OF(TYPE, FIELD) ((unsigned long)&(((TYPE *)0)->FIELD))
#define PSTRING_PTR(STR) ((STR)->str)

typedef struct pstring_t {
    unsigned len;
    char str[1];
} pstring_t;

static pstring_t *pstring_alloc(const char *t, unsigned len)
{
    pstring_t *str = (pstring_t *) malloc(sizeof(pstring_t) + len - 1);
    memcpy(PSTRING_PTR(str), t, len);
    return str;
}

static void pstring_delete(pstring_t *str)
{
    free(str);
}

static inline int starts_with_strcmp(const char *p, const char *text, unsigned len)
{
    return (strncmp(p, text, len) == 0);
}

static inline int starts_with_simple(const char *p, const char *text, unsigned len)
{
    const char *end = text + len;
    while (text < end) {
        if (*p++ != *text++) {
            return 0;
        }
    }
    return 1;
}

static inline int starts_with_avx2(const char *str, const char *text, unsigned len)
{
    uint64_t m, mask;
    __m256i s = _mm256_loadu_si256((const __m256i *)str);
    __m256i t = _mm256_loadu_si256((const __m256i *)text);
    assert(len <= 32);
    s = _mm256_cmpeq_epi8(s, t);
    m = _mm256_movemask_epi8(s);
    mask = ((uint64_t)1 << len) - 1;
    return ((m & mask) == mask);
}

int pstring_starts_with(const char *str, const char *text, unsigned len)
{
#ifdef __AVX2__
    if (len <= 32) {
        return starts_with_avx2(str, text, len);
    }
    else
#endif
    {
#if defined(USE_STRCMP)
        return starts_with_strcmp(str, text, len);
#else
        return starts_with_simple(str, text, len);
#endif
    }
}

#if 0
static int pstring_starts_with_(pstring_t *str, const char *text, unsigned len)
{
    const char *p = (const char *)PSTRING_PTR(str);
    return pstring_starts_with(p, text, len);
}

static int pstring_starts_with__(pstring_t *str1, pstring_t *str2)
{
    return pstring_starts_with_(str1, (const char *)PSTRING_PTR(str2), str2->len);
}
#endif

#ifdef PSTRING_C_TEST
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
    err += bench(starts_with_simple);
    puts("use strcmp");
    err += bench(starts_with_strcmp);
    if (err == 0) {
        puts("test ok");
    }
    return 0;
}
#endif /*pstring_C_TEST*/
