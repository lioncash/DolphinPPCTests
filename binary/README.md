The only difference between the two dols is that the Dolphin friendly one won't run any instructions with the OE bit set, since most instructions in Dolphin don't implement it (yet).

Essentially, run the Dolphin-friendly variant and extract the instruction_tests.txt file from sd.raw in Dolphin's User/Wii folder. Dolphin doesn't store contents that would otherwise go to the SD card directly on the host filesystem like with GCI folders for memory cards (unfortunately). So if you're on Windows, you'll need to use something like WinImage to extract it.

Once extracted, diff that text file with the instruction_tests_console.txt file in the same directory that this readme is in. If there's any differences, it means the emulator is not entirely accurate in terms of instruction behavior.
