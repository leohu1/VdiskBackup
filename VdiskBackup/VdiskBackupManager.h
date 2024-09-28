//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VDISKBACKUPMANAGER_H
#define VDISKBACKUP_VDISKBACKUPMANAGER_H

#include "common_util/filepath.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

struct VdiskBackupConfig{
public:
    cutl::filepath* source_path = nullptr;
    cutl::filepath* destination_path = nullptr;
    std::string id;
    std::vector<cutl::filepath> config_paths;
    size_t min_compact_size = 5;
    size_t buffer_size = 134217728;
    bool enable_fs_aware;
    bool enable_fs_agnostic;
    ~VdiskBackupConfig();
};

class VdiskBackupManager {
const std::string ConfigName = "VdiskBackupConfig.yaml";
private:
    std::map<std::string, VdiskBackupConfig> backup_configs;
public:
    VdiskBackupManager();
    void GetAllConfigs();
    void StartBackup();
};


#endif //VDISKBACKUP_VDISKBACKUPMANAGER_H
