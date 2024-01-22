#pragma once

#include <cstdarg>
#include <cstdlib>
#include <string>
#ifdef MACOS
#define nullptr NULL
#endif

inline std::string StringFromFormat(const char* format, ...)
{
    va_list args;
    char* buf = nullptr;

    va_start(args, format);
    vasprintf(&buf, format, args);
    va_end(args);

    std::string temp = buf;
    free(buf);
    return temp;
}

static char us[20];

inline char * upper(const char *s)
{
    char *u = us;
    if (s)
        for (; *s; s++, u++)
            *u = toupper(*s);
    *u = '\0';
    return us;
}
