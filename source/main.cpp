#include <cstdio>
#include <fat.h>
#include <gccore.h>
#include <sstream>
#include <string>
#include <sdcard/wiisd_io.h>
#include <sys/iosupport.h>
#include <vector>
#include <wiiuse/wpad.h>

#include "Tests.h"
#include "Utils.h"

static void* xfb = nullptr;
static GXRModeObj* rmode = nullptr;
static FILE* f = nullptr;

static ssize_t file_write(_reent* r, int fd, const char* ptr, size_t len)
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
    nullptr
};

// Initializes various system devices/capabilities.
static void Initialize()
{
    PAD_Init();
    VIDEO_Init();
    WPAD_Init();

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
    f = fopen("instruction_tests.txt", "w");
    if (f == nullptr)
    {
        printf("Unable to open: %s\n", path);
        return false;
    }

    return true;
}

// Checks if a button is down on any wiimote or GameCube controller.
static bool IsButtonDown(int gc_button_id, int wii_button_id)
{
    for (int i = 0; i < 4; i++)
    {
        const int gc_buttons_down = PAD_ButtonsDown(i);
        const int wii_buttons_down = WPAD_ButtonsDown(i);

        if (gc_buttons_down & gc_button_id || wii_buttons_down & wii_button_id)
            return true;
    }

    return false;
}

int main()
{
    Initialize();
    printf("Dolphin PPC Instruction Tests\n");
    printf("Press A to run PPC integer tests.\n");
    printf("Press Start or Home to exit.\n\n\n");

    // Keep the original stdout devoptab around, since 
    // we flip between writing to the file and displaying text on screen.
    const devoptab_t console_tab = *devoptab_list[STD_OUT];

    // Line buffered
    setvbuf(stdout, nullptr, _IOLBF, 0);

    while (true)
    {
        VIDEO_WaitVSync();
        PAD_ScanPads();
        WPAD_ScanPads();

        if (IsButtonDown(PAD_BUTTON_A, WPAD_BUTTON_A))
        {
            if (TryOpenFile("integer_tests.txt"))
            {
                printf("Running PPC Integer tests...\n");
                devoptab_list[STD_OUT] = &dotab_file;
                PPCIntegerTests();
                devoptab_list[STD_OUT] = &console_tab;
                fclose(f);
                printf("Done!\n");
            }
        }

        if (IsButtonDown(PAD_BUTTON_START, WPAD_BUTTON_HOME))
            exit(0);
    }

    return 0;
}
