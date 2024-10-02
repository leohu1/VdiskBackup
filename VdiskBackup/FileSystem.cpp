//
// Created by 26071 on 2024/9/15.
//

#include "FileSystem.h"
#include "indicators/progress_bar.hpp"
#include "md5.hpp"
#include "utils.h"
#include <thread>
#include <future>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <spdlog/spdlog.h>
#include <string>

using namespace indicators;
using namespace md5;

bool FileSystem::CopyFileWithProgressBar(const fs::path &srcPath, const fs::path &dstPath, std::string **pMd5,
                                         const std::string& prefix_text, const size_t buf_size) {
    // Hide cursor
    show_console_cursor(false);

    if (fs::exists(srcPath))
    {
        if (fs::exists(dstPath))
        {
            // remove if exists
            fs::remove(dstPath);
        }
        FILE *fr, *fw;
        DWORD result;
        if ((result = fopen_s(&fr, srcPath.string().c_str(), "rb")) != 0)
        {
            SPDLOG_ERROR("open file failed ({}), {}", result, srcPath.string());
            return false;
        }

        if ((result = fopen_s(&fw, dstPath.string().c_str(), "wb")) != 0)
        {
            SPDLOG_ERROR("open file failed ({}), {}", result, srcPath.string());
            return false;
        }

        auto* buffer = new uint8_t[buf_size];
        size_t read_size = 0;
        size_t write_size = 0;
        size_t total_size = GetFileSize(srcPath.string()).GetSizeBits();
        size_t total_num = total_size / buf_size;
        size_t write_num = 0;
        std::string size_string = FileSize(total_size).GetSizeString();

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
                option::End{"]"},
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

        while ((read_size = fread(buffer, 1, buf_size, fr)) > 0)
        {
            std::future<void> md5_async = std::async(std::launch::async,[&buffer, &md5, read_size](){
                md5_append(&md5, buffer, read_size);
            });
            write_size = fwrite(buffer, 1, read_size, fw);
            if (write_size != read_size)
            {
                SPDLOG_ERROR("write file failed, only write " +
                                        std::to_string(write_size) + ", read_size:" + std::to_string(read_size));
                return false;
            }
            write_num += 1;
            bar.set_progress(write_num);
            md5_async.wait();
        }
        // flush file to disk
        int ret = fflush(fw);
        if (0 != ret)
        {
            SPDLOG_ERROR("fail to flush file:" + dstPath.string());
            return false;
        }
        fclose(fw);
        fclose(fr);
        char digest[16];
        md5_finish(&md5, (md5_byte_t *)digest);
        std::string hash;
        hash.resize(16);
        std::copy(digest,digest+16,hash.begin());
        *pMd5 = new std::string;

        for (char & i : hash) {
            (**pMd5).push_back(hexval[((i >> 4) & 0xF)]);
            (**pMd5).push_back(hexval[i & 0x0F]);
        }
    }
    show_console_cursor(true);
    return true;
}

std::string FileSystem::GetFileMd5(const fs::path &path, const size_t buf_size) {
    FILE *fr;
    if (fopen_s(&fr, reinterpret_cast<const char *>(path.c_str()), "rb") != 0)
    {
        SPDLOG_ERROR("open file failed, " + path.string());
        return "";
    }
    auto* buffer = new uint8_t[buf_size];
    size_t read_size = 0;
    size_t total_size = GetFileSize(path.string()).GetSizeBits();
    size_t total_write_size = 0;
    size_t done_num = 0;
    size_t total_num = total_size / buf_size;
    std::string size_string = FileSize(total_size).GetSizeString();

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
            option::End{"]"},
            option::MaxProgress{total_num},
            option::ShowElapsedTime{true},
            option::ShowRemainingTime{true},
            option::Stream{std::cout},
    };
    std::cout << "Calculating MD5 for" << path.string() << std::endl;
    md5_state_t md5;
    md5_init(&md5);

    while ((read_size = fread(buffer, 1, buf_size, fr)) > 0)
    {
        md5_append(&md5, buffer, read_size);
        total_write_size += read_size;
        done_num += 1;
        bar.set_option(option::PostfixText{FileSize(total_write_size).GetSizeString() + "/" + size_string});
        bar.set_progress(done_num);
    }
    fclose(fr);
    char digest[16];
    md5_finish(&md5, (md5_byte_t *)digest);
    std::string hash;
    hash.resize(16);
    std::copy(digest,digest+16,hash.begin());
    std::string md5_s;

    for (char & i : hash) {
        md5_s.push_back(hexval[((i >> 4) & 0xF)]);
        md5_s.push_back(hexval[i & 0x0F]);
    }
    show_console_cursor(true);
    return md5_s;
}

std::string FileSystem::FileSize::GetSizeString() const {
    size_t size = file_bits;
    const std::string size_string[] = {"B", "KB", "MB", "GB", "TB"};
    for (const auto & i : size_string) {
        size_t num = size % 1024;
        if (size <= 1024){
            double d_size = ((double) size) + ((double) num / 1024.0);
            return std::format("{:.2f} {}", d_size, i);
        }
        size /= 1024;
    }
    return "";
}

FileSystem::FileSize::FileSize(size_t bits):file_bits(bits) {

}

size_t FileSystem::FileSize::GetSizeBits() const {
    return file_bits;
}

FileSystem::FileSize FileSystem::GetFileSize(const std::string& path) {
    HANDLE hFile;
    LARGE_INTEGER fr_size;
    hFile = CreateFileW(utils::charToLPWSTR(path.c_str()),               // file to open
                        GENERIC_READ,          // open for reading
                        FILE_SHARE_READ,       // share for reading
                        nullptr,                  // default security
                        OPEN_EXISTING,         // existing file only
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                        nullptr);
    if (hFile == INVALID_HANDLE_VALUE){
        DWORD errCode = GetLastError();
        SPDLOG_ERROR("get file size fail, get error code:" + std::to_string(errCode));
        return 0;
    }
    if (GetFileSizeEx(hFile, &fr_size) == 0){
        DWORD errCode = GetLastError();
        SPDLOG_ERROR("get file size fail, get error code:" + std::to_string(errCode));
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    return {static_cast<size_t>(fr_size.QuadPart)};
}

void FileSystem::PoolFileCopy::copyFileThreadFunc(void *arg)  {
    auto *para = (FileSystem::PoolFileCopy::copyThreadPara*)arg;
    HANDLE hSrc = CreateFile(
            para->srcFile.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if (hSrc == INVALID_HANDLE_VALUE) {
        return;
    }
    HANDLE hDst = CreateFile(
            para->destFile.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if (hDst == INVALID_HANDLE_VALUE) {
        return;
    }

    auto* buffer = new char[para->poolFileCopy->bufferSize];
    size_t offset = para->start * para->poolFileCopy->bufferSize;
    OVERLAPPED *src_overlapped, *dest_overlapped;
    src_overlapped = (OVERLAPPED*) malloc(sizeof(OVERLAPPED));
    dest_overlapped = (OVERLAPPED*) malloc(sizeof(OVERLAPPED));
    memset(src_overlapped, 0, sizeof(OVERLAPPED));
    memset(dest_overlapped, 0, sizeof(OVERLAPPED));
    src_overlapped->Offset = offset;
    dest_overlapped->Offset = offset;
    LARGE_INTEGER place;
    place.QuadPart = offset;
    SetFilePointerEx(hSrc, place, nullptr, FILE_BEGIN);
    SetFilePointerEx(hDst, place, nullptr, FILE_BEGIN);

    for (size_t i = 0;i < (para->end - para->start);++i){
        DWORD read_size;
        if (!ReadFile(hSrc, buffer, sizeof(buffer), &read_size, src_overlapped)) {
            // 错误处理
            CloseHandle(hSrc);
            return;
        }

        if (!WriteFile(hDst, buffer, read_size, nullptr, dest_overlapped)) {
            // 错误处理
            CloseHandle(hDst);
            return;
        }
        para->poolFileCopy->bar_tick();
    }
    delete[] buffer;
    CloseHandle(hDst);
    CloseHandle(hSrc);
}

FileSystem::PoolFileCopy::~PoolFileCopy() {
    pool->destroy_thread_pool();
    indicators::show_console_cursor(true);
}
FileSystem::PoolFileCopy::PoolFileCopy(size_t numThread) {
    indicators::show_console_cursor(false);
    pool = new thread_pool::thread_pool(numThread);
}
void FileSystem::PoolFileCopy::Copy(const fs::path &srcFile, const fs::path &destFile) {
    size_t fileSize = FileSystem::GetFileSize(srcFile.string()).GetSizeBits();
    HANDLE hDst = CreateFileW(
            utils::charToLPWSTR(destFile.string().c_str()),
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if (hDst == INVALID_HANDLE_VALUE) {
        LPCTSTR errMsg = nullptr;
        DWORD dw_error = GetLastError();
        errMsg = utils::ErrorMessage(dw_error);
        SPDLOG_ERROR("Get Error ({}): {}\n", dw_error, errMsg);
        return;
    }
    LARGE_INTEGER size;
    size.QuadPart = fileSize;
    SetFilePointerEx(hDst, size, nullptr, FILE_BEGIN);
    if (size.LowPart == INVALID_SET_FILE_POINTER && GetLastError()
                                                          != NO_ERROR)
    {
        SPDLOG_WARN("Error setting file size");
        return;
    }
    CloseHandle(hDst);
    size_t numThreads = 4;
    size_t total_chunk = fileSize / bufferSize;
    size_t each_thread = total_chunk / numThreads;

    bar = new ProgressBar(option::BarWidth{40},
                      option::Start{"["},
                      option::Fill{"="},
                      option::Lead{"="},
                      option::Remainder{"-"},
                      option::End{"]"},
                      option::ForegroundColor{Color::blue},
                      option::ShowElapsedTime{true},
                      option::ShowRemainingTime{true},
                      option::PrefixText{"Copying "+ srcFile.filename().string()},
                      option::MaxProgress{total_chunk},
                      option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}});

    for (size_t i = 0; i < numThreads; i++) {
        size_t start = i * each_thread;
        size_t end = start + each_thread;
        auto para = new copyThreadPara;
        para->poolFileCopy = this;
        para->srcFile = srcFile.string();
        para->destFile = destFile.string();
        para->start = start;
        para->end = end;
        pool->add_task(copyFileThreadFunc, (void*)para);
    }
}
void FileSystem::PoolFileCopy::bar_tick() {
    std::unique_lock<std::mutex> lock(bar_mutex);
    done_chuck++;
    bar->set_progress(done_chuck);
}
