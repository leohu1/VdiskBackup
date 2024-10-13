//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VOLUMESYSTEM_H
#define VDISKBACKUP_VOLUMESYSTEM_H
#include <Windows.h>
#include <string>
#include <vector>
using namespace std;

struct VolumeInfo{
public:
    string device_path;
    string GUID_path;
    string drive_letter;
};

class VolumeSystem {
public:
    static vector<VolumeInfo> GetAllVolumeInfo();
    static std::string GetVolumePaths(__in PWCHAR VolumeName);
    static size_t GetVolumeFreeSpace(std::string PathInVolume);
};


#endif //VDISKBACKUP_VOLUMESYSTEM_H
