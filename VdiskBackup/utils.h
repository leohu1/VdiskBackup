//
// Created by 26071 on 2024/9/28.
//

#ifndef VDISKBACKUP_UTILS_H
#define VDISKBACKUP_UTILS_H
#include <Windows.h>
#include "algorithm"
#include <iterator>
#include <string>

namespace utils{
    LPCTSTR ErrorMessage( DWORD error );

    LPWSTR charToLPWSTR(const char* charArray);

    std::string removeSpaces(const std::string& str);

    char* LPWSTRTochar(LPWSTR pWCStrKey);
}

#endif //VDISKBACKUP_UTILS_H
