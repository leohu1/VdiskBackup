//
// Created by 26071 on 2024/9/15.
//

#ifndef VDISKBACKUP_FILESYSTEM_H
#define VDISKBACKUP_FILESYSTEM_H

#include <string>
#include "common_util/filepath.h"
#include "common_util/fileutil.h"


class FileSystem {
public:
    static bool CopyFileWithProgressBar (const cutl::filepath &srcpath, const cutl::filepath &dstpath,
                                         const std::string& prefix_text = "", size_t buf_size = 8 * 1024);
    static std::string BuildSizeString (size_t size);
};


#endif //VDISKBACKUP_FILESYSTEM_H
