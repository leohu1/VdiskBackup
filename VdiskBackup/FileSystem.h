//
// Created by 26071 on 2024/9/15.
//

#ifndef VDISKBACKUP_FILESYSTEM_H
#define VDISKBACKUP_FILESYSTEM_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace FileSystem{
    class FileSize{
    private:
        size_t file_bits;
    public:
        explicit FileSize(size_t bits);
        std::string GetSizeString();
        [[nodiscard]] size_t GetSizeBits() const;
    };
    bool CopyFileWithProgressBar (const fs::path &srcPath, const fs::path &dstPath, std::string **pMd5,
                                         const std::string& prefix_text = "", size_t buf_size = 8 * 1024);
    FileSize GetFileSize(const std::string&);

    std::string GetFileMd5(const fs::path &path, size_t buf_size = 8 * 1024);
}


#endif //VDISKBACKUP_FILESYSTEM_H
