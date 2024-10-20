//
// Created by 26071 on 2024/10/6.
//

#ifndef VDISKBACKUP_BACKUPCONFIGMANAGER_H
#define VDISKBACKUP_BACKUPCONFIGMANAGER_H

#include "FileSystem.h"
#include "spdlog/async.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace fs = std::filesystem;

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

class BackupConfigManager {
public:
    static void WriteConfig(VdiskBackupConfig config);
    constexpr const static char ConfigName[] = "VdiskBackupConfig.yaml";
    constexpr const static char BackupResult[] = "VdiskBackupResult.yaml";
};


#endif//VDISKBACKUP_BACKUPCONFIGMANAGER_H
