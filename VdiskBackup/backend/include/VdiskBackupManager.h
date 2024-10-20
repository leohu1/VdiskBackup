//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VDISKBACKUPMANAGER_H
#define VDISKBACKUP_VDISKBACKUPMANAGER_H

#include "BackupConfigManager.h"
#include "FileSystem.h"
#include "spdlog/async.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace FileSystem;

class VdiskBackupManager {
private:
    std::map<std::string, VdiskBackupConfig> backup_configs;
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> ringbuffer_sink;
public:
    VdiskBackupManager();
    void GetAllConfigs();
    void StartBackup();
    void Init();
    void CleanUp();

private:
    std::map<std::string, std::string> GetLastMd5(fs::path path);
};


#endif //VDISKBACKUP_VDISKBACKUPMANAGER_H
