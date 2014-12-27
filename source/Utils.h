#pragma once

#include <cstdarg>
#include <cstdlib>
#include <string>

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
