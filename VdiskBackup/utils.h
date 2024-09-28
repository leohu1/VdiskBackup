//
// Created by 26071 on 2024/9/28.
//

#ifndef VDISKBACKUP_UTILS_H
#define VDISKBACKUP_UTILS_H
#include <Windows.h>

LPCTSTR ErrorMessage( DWORD error )

// Routine Description:
//      Retrieve the system error message for the last-error code
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

LPWSTR charToLPWSTR(const char* charArray) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, charArray, -1, NULL, 0);
    auto wstrTo = new WCHAR[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wstrTo, size_needed);
    return wstrTo;
}

#endif //VDISKBACKUP_UTILS_H
