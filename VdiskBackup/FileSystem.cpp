//
// Created by 26071 on 2024/9/15.
//

#include "FileSystem.h"
#include "common_util/inner/filesystem.h"
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <log4cplus/configurator.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <Windows.h>
#include <string>
using namespace log4cplus;
using namespace indicators;

LPWSTR charToLPWSTR(const char* charArray) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, charArray, -1, NULL, 0);
    auto wstrTo = new WCHAR[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wstrTo, size_needed);
    return wstrTo;
}

bool FileSystem::CopyFileWithProgressBar(const cutl::filepath &srcpath, const cutl::filepath &dstpath,
                                         const std::string& prefix_text, const size_t buf_size) {
    // Hide cursor
    show_console_cursor(false);

    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));
    if (srcpath.isfile())
    {
        if (dstpath.exists())
        {
            // remove if exists
            removefile(dstpath);
        }
        FILE *fr, *fw;
        if (fopen_s(&fr, srcpath.str().c_str(), "rb") != 0)
        {
            LOG4CPLUS_ERROR(logger, ("open file failed, " + srcpath.str()).c_str());
            return false;
        }
        cutl::file_guard frd(fr);

        if (fopen_s(&fw, dstpath.str().c_str(), "wb") != 0)
        {
            LOG4CPLUS_ERROR(logger, ("open file failed, " + srcpath.str()).c_str());
            return false;
        }
        cutl::file_guard fwt(fw);

        auto* buffer = new uint8_t[buf_size];
        size_t read_size = 0;
        size_t write_size = 0;
        size_t total_size = 0;
        size_t total_write_size = 0;
        fseek(fr,0, SEEK_END);
        long total_size_l = ftell(fr);
        if (total_size_l < 0){
            HANDLE hFile;
            LARGE_INTEGER fr_size;
            hFile = CreateFile(charToLPWSTR(srcpath.str().c_str()),               // file to open
                               GENERIC_READ,          // open for reading
                               FILE_SHARE_READ,       // share for reading
                               NULL,                  // default security
                               OPEN_EXISTING,         // existing file only
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                               NULL);
            if (hFile == INVALID_HANDLE_VALUE){
                DWORD errCode = GetLastError();
                LOG4CPLUS_ERROR(logger, ("get file size fail, get error code:" + std::to_string(errCode)).c_str());
                return false;
            }
            if (GetFileSizeEx(hFile, &fr_size) == 0){
                DWORD errCode = GetLastError();
                LOG4CPLUS_ERROR(logger, ("get file size fail, get error code:" + std::to_string(errCode)).c_str());
                return false;
            }
            total_size = fr_size.QuadPart;
        }else{
            total_size = total_size_l;
        }
        fseek(fr,0, SEEK_SET);
        size_t total_num = total_size / buf_size;
        size_t write_num = 0;
        std::string size_string = BuildSizeString(total_size);
        ProgressBar bar{
                option::BarWidth{50},
                option::Start{"["},
                option::Fill{"■"},
                option::Lead{"■"},
                option::Remainder{"-"},
                option::End{" ]"},
                option::ForegroundColor{Color::cyan},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
                option::MaxProgress{total_num},
                option::ShowElapsedTime{true},
                option::ShowRemainingTime{true},
        };
        if (prefix_text.empty() == 0){
            bar.set_option(option::PrefixText{prefix_text});
        }
        while ((read_size = fread(buffer, 1, buf_size, frd.getfd())) > 0)
        {
            write_size = fwrite(buffer, 1, read_size, fwt.getfd());
            if (write_size != read_size)
            {
                LOG4CPLUS_ERROR(logger,("write file failed, only write " +
                                        std::to_string(write_size) + ", read_size:" + std::to_string(read_size)).c_str());
                return false;
            }
            write_num += 1;
            total_write_size += write_size;
            bar.set_option(option::PostfixText{BuildSizeString(total_write_size) + "/" + size_string});
            bar.set_progress(write_num);
        }
        // flush file to disk
        int ret = fflush(fwt.getfd());
        if (0 != ret)
        {
            LOG4CPLUS_ERROR(logger, ("fail to flush file:" + dstpath.str()).c_str());
            return false;
        }
        if (!cutl::file_sync(fwt.getfd()))
        {
            LOG4CPLUS_ERROR(logger, ("file_sync failed for " + dstpath.str()).c_str());
            return false;
        }
    }
    show_console_cursor(true);
    return true;
}

std::string FileSystem::BuildSizeString(size_t size) {
    const std::string size_string[] = {"B", "KB", "MB", "GB", "TB"};
    for (const auto & i : size_string) {
        size_t num = size % 1024;
        size /= 1024;
        if (size <= 0){
            return std::to_string(num) + i + " ";
        }
    }
    return "";
}
