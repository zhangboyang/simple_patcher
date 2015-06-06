#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "md5.h"

#define MAXMD5LEN 33
#define MAXBUFSIZE 4096
#define MAXFILELEN (1024 * 1024 * 32)

void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    putchar('\n');
    exit(1);
    va_end(ap);
}

int main(int argc, char *argv[])
{
    static char buf[MAXBUFSIZE];
    static char data[MAXFILELEN];
    static char md5str[MAXMD5LEN];
    char *p;
    FILE *fp, *sfp;
    int len, ptr, diff, flag, a, b, c;
    
    if (argc == 1) {
        sfp = stdin;
    } else {
        if (argc != 2)
            fail("usage: %s [script_file_name]", argv[0]);
        sfp = fopen(argv[1], "r");
        if (!sfp) fail("can't open script file: %s", argv[1]);
    }
    
    while (fscanf(sfp, "%s", buf) == 1) {
        //printf("==> %s\n", buf);
        if (strncmp(buf, "/*", 2) == 0) {
            while (fscanf(sfp, "%s", buf) == 1)
                if (strstr(buf, "*/"))
                    break;
        } else if (*buf == '#' || *buf == ';' || strncmp(buf, "//", 2) == 0) {
            fscanf(sfp, "%*[^\n]");
        } else if (*buf == '?') {
            if (md5(data, len, md5str) < 0)
                fail("can't calculate md5");
            flag = 0;
            p = buf + 1;
            while (1) {
                if (strnicmp(md5str, p, strlen(md5str)) == 0) {
                    flag = 1;
                    break;
                }
                p = strstr(p, "||");
                if (!p) break;
                p += 2;
            }
            if (!flag) fail("md5sum mismatch");
        } else if (*buf == '<') {
            fp = fopen(buf + 1, "rb");
            if (!fp) fail("can't open input file: %s", buf + 1);
            len = fread(data, 1, MAXFILELEN, fp);
            if (len <= 0) fail("read failed", buf + 1);
            if (len >= MAXFILELEN) fail("data buffer is too small");
            fclose(fp);
        } else if (*buf == '>') {
            fp = fopen(buf + 1, "wb");
            if (!fp) fail("can't open output file: %s", buf + 1);
            if (fwrite(data, 1, len, fp) != len) fail("write failed");
            fclose(fp);
        } else if (*buf == ':') {
            if (sscanf(buf, ":%x/%x", &a, &b) == 2) {
                diff = a - b;
                ptr = a;
            } else if (sscanf(buf, ":/%x", &b) == 1) {
                ptr = b + diff;
            } else if (sscanf(buf, ":%x", &a) == 1) {
                ptr = a;
            } else
                fail("invalid position instruction: %s", buf);
        } else if (sscanf(buf, "%x", &c) == 1) {
            if (c < 0 || c > 0xff) fail("invalid byte: %s", buf);
            data[ptr++] = c;
        } else
            fail("unrecognized instruction: %s", buf);
    }
    
    return 0;
}
