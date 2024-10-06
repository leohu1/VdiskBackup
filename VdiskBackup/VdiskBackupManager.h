//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VDISKBACKUPMANAGER_H
#define VDISKBACKUP_VDISKBACKUPMANAGER_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>
#include "FileSystem.h"
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace fs = std::filesystem;
using namespace FileSystem;

struct VdiskBackupConfig{
public:
    fs::path source_path;
    fs::path destination_path;
    std::string id;
    std::vector<fs::path> config_paths;
    size_t min_compact_size = 5;
    size_t min_merge_size = 5;
    size_t buffer_size = 134217728;
    bool enable_fs_aware;
    bool enable_fs_agnostic;
    bool enable_merge;
    ~VdiskBackupConfig();
};

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
    constexpr const static char ConfigName[] = "VdiskBackupConfig.yaml";
    constexpr const static char BackupResult[] = "VdiskBackupResult.yaml";
private:
    std::map<std::string, std::string> GetLastMd5(fs::path path);
};


#endif //VDISKBACKUP_VDISKBACKUPMANAGER_H
