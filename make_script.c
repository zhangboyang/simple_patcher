#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
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

int read_file(char *buf, const char *fn)
{
    int len;
    FILE *fp = fopen(fn, "rb");
    if (!fp) fail("can't open file: %s", fn);
    len = fread(buf, 1, MAXFILELEN, fp);
    if (len <= 0) fail("read failed");
    if (len >= MAXFILELEN) fail("data buffer is too small");
    fclose(fp);
    return len;
}

int main(int argc, char *argv[])
{
    static char buf[MAXBUFSIZE];
    static char data1[MAXFILELEN];
    static char data2[MAXFILELEN];
    static char md5str1[MAXMD5LEN];
    static char md5str2[MAXMD5LEN];
    time_t timestamp;
    int len1, len2;
    int i, cnt;
    
    if (argc != 3)
        fail("usage: %s [old_file] [new_file]", argv[0]);
    
    len1 = read_file(data1, argv[1]);
    len2 = read_file(data2, argv[2]);
    if (md5(data1, len1, md5str1) < 0 || md5(data2, len2, md5str2) < 0)
        fail("can't calculate md5");
    
    if (len1 != len2) fail("file length mismatch");
    
    if (memcmp(data1, data2, len1) == 0) fail("file content is same");

    timestamp = time(NULL);  
    strftime(buf, MAXBUFSIZE, "%Y/%m/%d %H:%M:%S", localtime(&timestamp));
    printf("# patch script: %s -> %s\n", argv[1], argv[2]);
    printf("# generated at %s \n", buf);
    printf("\n");
    printf("# read old file\n");
    printf("<%s\n", argv[1]);
    printf("\n");
    printf("# check input md5sum\n");
    printf("?%s\n", md5str1);
    printf("\n");
    
    for (i = 0; i < len1; i++)
        if (data1[i] != data2[i]) {
            printf(":%x\n", i);
            cnt = 0;
            while (i < len1 && data1[i] != data2[i]) {
                printf("%02X ", (int)data2[i] & 0xff);
                cnt++;
                if (cnt == 16) {
                    printf("\n");
                    cnt = 0;
                }
                i++;
            }
            printf("\n");
            if (cnt) printf("\n");
        }
    
    printf("# check output md5sum\n");
    printf("?%s\n", md5str2);
    printf("\n");
    printf("# write new file\n");
    printf(">script_%s\n", argv[2]);
    return 0;
}
