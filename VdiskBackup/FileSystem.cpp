//
// Created by 26071 on 2024/9/15.
//

#include "FileSystem.h"
#include "common_util/inner/filesystem.h"
#include <indicators/progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <string>
#include "md5.hpp"
#include "utils.h"

using namespace indicators;
using namespace md5;

bool FileSystem::CopyFileWithProgressBar(const cutl::filepath &srcpath, const cutl::filepath &dstpath,
                                         const std::string& prefix_text, const size_t buf_size) {
    // Hide cursor
    show_console_cursor(false);

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
            SPDLOG_ERROR("open file failed, " + srcpath.str());
            return false;
        }
        cutl::file_guard frd(fr);

        if (fopen_s(&fw, dstpath.str().c_str(), "wb") != 0)
        {
            SPDLOG_ERROR("open file failed, " + srcpath.str());
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
            hFile = CreateFileW(charToLPWSTR(srcpath.str().c_str()),               // file to open
                               GENERIC_READ,          // open for reading
                               FILE_SHARE_READ,       // share for reading
                               nullptr,                  // default security
                               OPEN_EXISTING,         // existing file only
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                               nullptr);
            if (hFile == INVALID_HANDLE_VALUE){
                DWORD errCode = GetLastError();
                SPDLOG_ERROR("get file size fail, get error code:" + std::to_string(errCode));
                return false;
            }
            if (GetFileSizeEx(hFile, &fr_size) == 0){
                DWORD errCode = GetLastError();
                SPDLOG_ERROR("get file size fail, get error code:" + std::to_string(errCode));
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

        // Set output mode to handle virtual terminal sequences
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE)
        {
            SPDLOG_WARN("get console mode fail, get error code:" + std::to_string(GetLastError()));
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode))
        {
            SPDLOG_WARN("get console mode fail, get error code:" + std::to_string(GetLastError()));
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode))
        {
            SPDLOG_WARN("set console mode fail, get error code:" + std::to_string(GetLastError()));
        }

        ProgressBar bar{
                option::BarWidth{50},
                option::Start{"["},
                option::Fill{"="},
                option::Lead{"="},
                option::Remainder{"-"},
                option::End{" ]"},
                option::MaxProgress{total_num},
                option::ShowElapsedTime{true},
                option::ShowRemainingTime{true},
                option::Stream{std::cout},
        };
        if (prefix_text.empty() == 0){
            std::cout << prefix_text << std::endl;
        }
        md5_state_t md5;
        md5_init(&md5);

        while ((read_size = fread(buffer, 1, buf_size, frd.getfd())) > 0)
        {
            write_size = fwrite(buffer, 1, read_size, fwt.getfd());
            md5_append(&md5, buffer, read_size);
            if (write_size != read_size)
            {
                SPDLOG_ERROR("write file failed, only write " +
                                        std::to_string(write_size) + ", read_size:" + std::to_string(read_size));
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
            SPDLOG_ERROR("fail to flush file:" + dstpath.str());
            return false;
        }
        if (!cutl::file_sync(fwt.getfd()))
        {
            SPDLOG_ERROR("file_sync failed for " + dstpath.str());
            return false;
        }
        char digest[16];
        md5_finish(&md5, (md5_byte_t *)digest);
        std::string hash;
        hash.resize(16);
        std::copy(digest,digest+16,hash.begin());
        std::string hex;

        for (size_t i = 0; i < hash.size(); i++) {
            hex.push_back(hexval[((hash[i] >> 4) & 0xF)]);
            hex.push_back(hexval[(hash[i]) & 0x0F]);
        }

        FILE *fmd;
        if (fopen_s(&fmd, (dstpath.str()+".md5").c_str(), "w") != 0)
        {
            SPDLOG_ERROR("open file failed, " + srcpath.str());
            return false;
        }
        fprintf(fmd,"%s", hex.c_str());
        fclose(fmd);
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
