# Dolphin PPC CPU Tests

Just a little utility I use to test CPU instructions on Dolphin. This dumps all tested instruction results to
text files on the SD card (e.g. "integer_tests.txt", "floating_point_tests.txt", etc).

This can also potentially be used for other PowerPC-based emulators as well, although the Makefile and code in main.cpp will need to be modified.

## How to use it (on the Wii)
1. Run it on the console and select the tests to run.
2. It'll dump the results to the SD card.
3. Run it in Dolphin, it'll dump the resulting text files to the virtual SD card (make sure to set the SD card as inserted in the Config -> Wii menu).
4. As of writing, Dolphin doesn't have an easy way to access the SD card contents.
Currently you'll need to use an external tool (if on Windows) to access the virtual SD card's contents
(I've only tested with WinImage, and that seems to work).
5. Diff the test files with each other.
6. If any values differ from the hardware results, your PowerPC emulation is inaccurate.

## Building it

1. Install devkitPPC
2. Just run make.
