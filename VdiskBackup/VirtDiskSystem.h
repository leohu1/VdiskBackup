#include <string>
#include <vector>
#include <deque>

#ifndef VDISKBACKUP_VIRTDISKSYSTEM_H
#define VDISKBACKUP_VIRTDISKSYSTEM_H


namespace VirtDiskSystem{
    bool CompactVdiskFileSystemAware(std::string path);
    bool CompactVdiskFileSystemAgnostic(std::string path);
    std::string GetVdiskParent(std::string path);
    std::vector<std::string> GetVdiskParents(const std::string& path);
    bool MergeVdiskToParent(std::string path);
};

#endif //VDISKBACKUP_VIRTDISKSYSTEM_H
