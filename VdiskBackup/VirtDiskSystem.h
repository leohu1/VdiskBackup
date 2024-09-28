#include <string>

#ifndef VDISKBACKUP_VIRTDISKSYSTEM_H
#define VDISKBACKUP_VIRTDISKSYSTEM_H

class VirtDiskSystem{
    public: 
    static bool CompactVdiskFileSystemAware(std::string path);
};

#endif //VDISKBACKUP_VIRTDISKSYSTEM_H
