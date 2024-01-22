#include <cstdio>
#include <fat.h>
#include <gccore.h>
#include <sys/iosupport.h>

#include "Tests.h"
#include "Utils.h"

static void* xfb = nullptr;
static GXRModeObj* rmode = nullptr;
static FILE* f = nullptr;

static ssize_t file_write(_reent*, void*, const char* ptr, size_t len)
{
    if (len > 1)
        fprintf(f, "%.*s", len, ptr);

    return len;
}

static const devoptab_t dotab_file = {
    "file",
    0,
    nullptr,
    nullptr,
    file_write,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

// Initializes various system devices/capabilities.
static void Initialize()
{
    VIDEO_Init();

    if (!fatInitDefault())
    {
        printf("Unable to initialize FAT subsystem. Exiting...\n");
        exit(0);
    }

    // Initialize the XFB
    rmode = VIDEO_GetPreferredMode(nullptr);
    void* framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode&VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    xfb = framebuffer;
}

static bool TryOpenFile(const char* path)
{
    f = fopen(path, "w");
    if (f == nullptr)
    {
        printf("Unable to open: %s\n", path);
        return false;
    }

    return true;
}

int main()
{
    Initialize();
    printf("Dolphin PPC Instruction Tests\n");
    printf("Will exit when done.\n");

    devoptab_list[STD_OUT] = &dotab_file;

    // Line buffered
    setvbuf(stdout, nullptr, _IOLBF, 0);

    if (TryOpenFile("instruction_tests.txt"))
    {
        PPCIntegerTests();
        PPCFloatingPointTests();
        PPCConditionRegisterTests();
        fclose(f);
    }

    // Exit is required.
    exit(0);
    return 0;
}
