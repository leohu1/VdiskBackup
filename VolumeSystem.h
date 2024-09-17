//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VOLUMESYSTEM_H
#define VDISKBACKUP_VOLUMESYSTEM_H
#include <vector>
#include <string>
#include <windows.h>
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
};


#endif //VDISKBACKUP_VOLUMESYSTEM_H
