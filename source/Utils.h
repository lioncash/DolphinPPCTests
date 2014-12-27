#pragma once

#include <cstdarg>
#include <cstdlib>
#include <stdio.h>
#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        elems.push_back(item);

    return elems;
}

inline std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

inline std::string& trim_left_in_place(std::string& str)
{
    size_t i = 0;

    while (i < str.size() && isspace(str[i]))
        i++;

    return str.erase(0, i);
}

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