//
// Created by 26071 on 2024/9/15.
//

#ifndef VDISKBACKUP_FILESYSTEM_H
#define VDISKBACKUP_FILESYSTEM_H

#include <string>
#include "common_util/filepath.h"
#include "common_util/fileutil.h"

namespace FileSystem{
    class FileSize{
    private:
        size_t file_bits;
    public:
        FileSize(size_t bits);
        std::string GetSizeString();
        size_t GetSizeBits() const;
    };
    bool CopyFileWithProgressBar (const cutl::filepath &srcpath, const cutl::filepath &dstpath,
                                         const std::string& prefix_text = "", size_t buf_size = 8 * 1024);
    FileSize GetFileSize(const std::string&);
}


#endif //VDISKBACKUP_FILESYSTEM_H
