//
// Created by 26071 on 2024/9/16.
//

#include "VdiskBackupManager.h"
#include "FileSystem.h"
#include "VirtDiskSystem.h"
#include "VolumeSystem.h"
#include "common_util/filepath.h"
#include "utils.h"
#include "yaml-cpp/yaml.h"
#include <spdlog/spdlog.h>

VdiskBackupManager::VdiskBackupManager() {
    SPDLOG_INFO("VdiskBackupManager initialized");
}

void VdiskBackupManager::GetAllConfigs() {
    // 获取全部拷贝文件，存储为 VdiskBackupConfig
    std::vector<VolumeInfo> vec = VolumeSystem::GetAllVolumeInfo();

    for (auto & it : vec) {
        if (!it.drive_letter.empty()){
            cutl::filepath base_path = cutl::filepath(utils::removeSpaces(it.drive_letter));
            cutl::filepath path = base_path.join(VdiskBackupManager::ConfigName);
            if (path.exists()){
                SPDLOG_INFO("get config at " + path.abspath());
                YAML::Node configs_data = YAML::LoadFile(path.abspath());
                YAML::Node backups = configs_data["backups"];
                if (backups && backups.IsSequence()){
                    for (auto && i : backups) {
                        if (i["id"]){
                            auto id = i["id"].as<string>();
                            VdiskBackupConfig config;
                            if (backup_configs.contains(id)){
                                config = backup_configs[id];
                            }else{
                                config = VdiskBackupConfig();
                            }
                            config.config_paths.push_back(path);
                            if (i["source"]){
                                config.source_path = new cutl::filepath(
                                        base_path.str() + cutl::filepath::separator() + i["source"].as<string>());
                            }
                            if (i["destination"]){
                                config.destination_path = new cutl::filepath(
                                        base_path.str() + cutl::filepath::separator() + i["destination"].as<string>());
                            }
                            if (i["min_compact_size"]){
                                config.min_compact_size = i["min_compact_size"].as<size_t>() * 1024 * 1024 *1024;
                            }
                            if (i["copy_buffer_size"]){
                                config.buffer_size = i["copy_buffer_size"].as<size_t>();
                            }
                            if (i["enable_file_system_aware_compact"]){
                                config.enable_fs_aware = i["enable_file_system_aware_compact"].as<bool>();
                            }
                            if (i["enable_file_system_agnostic_compact"]){
                                config.enable_fs_agnostic = i["enable_file_system_agnostic_compact"].as<bool>();
                            }
                            backup_configs[id] = config;
                        }
                    }
                }
            }
        }
    }
}

void VdiskBackupManager::StartBackup() {
    for (const auto& i : backup_configs){
        VdiskBackupConfig config = i.second;
        if (config.source_path->exists() && config.enable_fs_aware){
            if (FileSystem::GetFileSize(config.source_path->abspath()).GetSizeBits() >= config.min_compact_size){
                VirtDiskSystem::CompactVdiskFileSystemAware(config.source_path->abspath());
            }
        }
        if (config.source_path->exists() && config.enable_fs_agnostic){
            if (FileSystem::GetFileSize(config.source_path->abspath()).GetSizeBits() >= config.min_compact_size){
                VirtDiskSystem::CompactVdiskFileSystemAgnostic(config.source_path->abspath());
            }
        }
    }
}

VdiskBackupConfig::~VdiskBackupConfig() {
    if (source_path != nullptr){
        delete source_path;
        source_path = nullptr;
    }
    if (destination_path != nullptr){
        delete destination_path;
        destination_path = nullptr;
    }
}
