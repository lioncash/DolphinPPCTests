#include <cstdio>
#include <gccore.h>
#include <sstream>
#include <string>
#include <vector>

#include "Tests.h"
#include "Utils.h"

// Sort of gross.
std::string buffer;

static void* xfb = nullptr;
static GXRModeObj* rmode = NULL;

void* Initialize()
{
    VIDEO_Init();
    PAD_Init();

    rmode = VIDEO_GetPreferredMode(nullptr);
    void* framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode&VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    return framebuffer;
}

int main()
{
    xfb = Initialize();

    // Do our tests (appends to the buffer string above (hence why it's kind of gross)).
    PPCIntegerTests();
    printf(buffer.c_str());

    while (true)
    {
        VIDEO_WaitVSync();
        PAD_ScanPads();

        int buttonsDown = PAD_ButtonsDown(0);
        
        if (buttonsDown & PAD_BUTTON_START)
            exit(0);
    }

    return 0;
}
