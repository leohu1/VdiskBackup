#include <string>
#include <vector>
#include <deque>

#ifndef VDISKBACKUP_VIRTDISKSYSTEM_H
#define VDISKBACKUP_VIRTDISKSYSTEM_H


namespace VirtDiskSystem{
    enum VdiskCompactOption{
        FSAware,
        FSAgnostic
    };
    bool CompactVdisk(std::string path, VdiskCompactOption option);
    std::string GetVdiskParent(std::string path);
    std::vector<std::string> GetVdiskParents(const std::string& path);
    bool MergeVdiskToParent(std::string path);
    bool BuildChildVdisk(std::string path, std::string parent_path);
};

#endif //VDISKBACKUP_VIRTDISKSYSTEM_H
