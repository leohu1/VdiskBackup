//
// Created by 26071 on 2024/9/15.
//

#ifndef VDISKBACKUP_FILESYSTEM_H
#define VDISKBACKUP_FILESYSTEM_H

#include "indicators/dynamic_progress.hpp"
#include "indicators/progress_bar.hpp"
#include "thread_pool.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace fs = std::filesystem;
using namespace indicators;

namespace FileSystem{
    class FileSize{
    private:
        size_t file_bits;
    public:
        FileSize(size_t bits);
        std::string GetSizeString() const;
        [[nodiscard]] size_t GetSizeBits() const;
    };
    bool CopyFileWithProgressBar (const fs::path &srcPath, const fs::path &dstPath, std::string **pMd5,
                                         const std::string& prefix_text = "", size_t buf_size = 8 * 1024);
    class PoolFileCopy{
        struct copyThreadPara{
            std::string srcFile;
            std::string destFile;
            size_t start;
            size_t end;
            PoolFileCopy *poolFileCopy;
        };
        thread_pool::thread_pool *pool;
        static void copyFileThreadFunc(void *arg);
        size_t bufferSize = 1024*1024*128;
        ProgressBar *bar = nullptr;
        size_t done_chuck = 0;
        std::mutex bar_mutex;
        void bar_tick();
    public:
        void Copy(const fs::path& srcFile, const fs::path& destFile);
        explicit PoolFileCopy(size_t numThread=4);
        ~PoolFileCopy();
    };
    FileSize GetFileSize(const std::string&);

    std::string GetFileMd5(const fs::path &path, size_t buf_size = 8 * 1024);
}


#endif //VDISKBACKUP_FILESYSTEM_H
