#ifndef _KERL__STDIO_H_
#define _KERL__STDIO_H_

#define EXTERN extern

#define O_CREAT 1
#define O_RDWR  2

#define MAX_PATH 128

#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp) \
    do { \
        if (!(exp)) \
            assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__); \
    } while (0)
#else
#define assert(exp) \
    do {} while (0)
#endif

PUBLIC int open(const char *pathname, int flags);
PUBLIC int close(int fd);

#endif
