#include <string>
#include <vector>

#ifndef VDISKBACKUP_VIRTDISKSYSTEM_H
#define VDISKBACKUP_VIRTDISKSYSTEM_H


namespace VirtDiskSystem{
    bool CompactVdiskFileSystemAware(std::string path);
    bool CompactVdiskFileSystemAgnostic(std::string path);
    std::string GetVdiskParents(std::string path);
};

#endif //VDISKBACKUP_VIRTDISKSYSTEM_H
