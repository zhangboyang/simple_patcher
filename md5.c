#include <stdio.h>
#include <windows.h>

typedef struct {
    ULONG i[2];
    ULONG buf[4];
    unsigned char in[64];
    unsigned char digest[16];
} MD5_CTX;

typedef void (CALLBACK *MD5Init_t)(MD5_CTX*);
typedef void (CALLBACK *MD5Update_t)(MD5_CTX*, unsigned char*, unsigned int);
typedef void (CALLBACK *MD5Final_t)(MD5_CTX*);

static int init_flag = 0;
static MD5Init_t MD5Init;
static MD5Update_t MD5Update;
static MD5Final_t MD5Final;

static void init()
{
    HMODULE hModule = LoadLibrary("cryptdll.dll");
    if (!hModule) return;
    MD5Init = (void *) GetProcAddress(hModule, "MD5Init");
    MD5Update = (void *) GetProcAddress(hModule, "MD5Update");
    MD5Final = (void *) GetProcAddress(hModule, "MD5Final");
    if (!MD5Init || !MD5Update || !MD5Final) {
        FreeLibrary(hModule);
        return;
    }
    init_flag = 1;
}

int md5(const char *buf, int len, char *out)
{
    MD5_CTX mc;
    int i;
    if (!init_flag) init();
    if (!init_flag) return -1;
    
    MD5Init(&mc);
    MD5Update(&mc, (unsigned char *) buf, (unsigned int) len);
    MD5Final(&mc);
    
    for (i = 0; i < 16; i++)
        sprintf(out + i * 2, "%02x", (int) mc.digest[i] & 0xff);
    
    return 0;
}
