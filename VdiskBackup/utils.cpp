//
// Created by 26071 on 2024/9/28.
//

#include "utils.h"

LPCTSTR utils::ErrorMessage( DWORD error )
{

    LPVOID lpMsgBuf;

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, nullptr );

    return((LPCTSTR)lpMsgBuf);
}

LPWSTR utils::charToLPWSTR(const char* charArray) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, charArray, -1, NULL, 0);
    auto wstrTo = new WCHAR[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wstrTo, size_needed);
    return wstrTo;
}

std::string utils::removeSpaces(const std::string& str) {
    std::string result;
    std::copy_if(str.begin(), str.end(), std::back_inserter(result), [](char c) {
        return !std::isspace(c);
    });
    return result;
}