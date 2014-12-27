#include <grrlib.h>

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

#include "BitmapFont.h"
#include "Tests.h"
#include "Utils.h"

// Sort of gross.
std::string buffer;

// Formatted printing that obeys the '\n' character. GRRLIB's print does not actually handle this correctly.
static void FormattedTextPrint(int initial_x, int initial_y, GRRLIB_texImg* const bitmap, int font_height, int font_color, float zoom)
{
    std::vector<std::string> vec = split(buffer, '\n');

    for (std::string& str : vec)
    {
        trim_left_in_place(str);
        GRRLIB_Printf(initial_x, initial_y, bitmap, font_color, zoom, str.c_str());

        // + 2 is used as a little bit of leeway in terms of vertical line separation.
        // This way the text lines don't look like they're stacked on top of each other. 
        initial_y += font_height + 2;
    }
}

int main()
{
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wii Remotes
    WPAD_Init();

    // Initialize the bitmap font.
    GRRLIB_texImg* bitmap_texture = GRRLIB_LoadTexture(bitmap_font);
    GRRLIB_InitTileSet(bitmap_texture, 8, 16, 0);

    // Black background
    GRRLIB_SetBackgroundColour(0x00, 0x00, 0x00, 0xFF);

    // Do our tests (appends to the buffer string above (hence why it's kind of gross)).
    PPCIntegerTests();

    while (true)
    {
        FormattedTextPrint(20, 20, bitmap_texture, 16, 0xFFFFFFFF, 1);

        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        // Render the frame buffer to the TV
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(bitmap_texture);
    GRRLIB_Exit();

    // GRRLIB: Use exit() to exit a program, do not use 'return' from main()
    exit(0);
}
