#ifndef _KERL__STDIO_H_
#define _KERL__STDIO_H_

#include "const.h"

#define EXTERN extern

#define STR_DEFAULT_LEN 1024

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

struct stat {
    int st_dev;
    int st_ino;
    int st_mode;
    int st_rdev;
    int st_size;
};

PUBLIC int printf(const char *fmt, ...);
PUBLIC int printl(const char *fmt, ...);

PUBLIC int open(const char *pathname, int flags);
PUBLIC int close(int fd);
PUBLIC int write(int fd, const void *buf, int count);
PUBLIC int read(int fd, void *buf, int count);
PUBLIC int unlink(const char *pathname);

PUBLIC int fork();
PUBLIC int getpid();
PUBLIC int wait(int *status);
PUBLIC void exit(int status);

PUBLIC int exec(const char *path);
PUBLIC int execl(const char *path, const char *arg, ...);
PUBLIC int execv(const char *path, char *argv[]);

PUBLIC int stat(const char *path, struct stat *buf);

#endif
