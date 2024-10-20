//
// Created by 26071 on 2024/10/6.
//

#include "BackupConfigManager.h"


void BackupConfigManager::WriteConfig(VdiskBackupConfig config){
    YAML::Emitter source_out;
    source_out << YAML::BeginMap << YAML::Key << "backups" << YAML::BeginSeq << YAML::BeginMap;
    source_out << YAML::Key << "id" << YAML::Value << "A";
    source_out << YAML::Key << "source" << YAML::Value << fs::relative(config.source_path, config.source_path.root_path()).string();
    source_out << YAML::Key << "min_compact_size" << YAML::Value << config.min_compact_size;
    source_out << YAML::Key << "min_merge_size" << YAML::Value << config.min_merge_size;
    source_out << YAML::Key << "copy_buffer_size" << YAML::Value << config.buffer_size;
    source_out << YAML::Key << "enable_file_system_aware_compact" << YAML::Value << config.enable_fs_aware;
    source_out << YAML::Key << "enable_file_system_agnostic_compact" << YAML::Value << config.enable_fs_agnostic;
    source_out << YAML::Key << "enable_merge" << YAML::Value << config.enable_merge;
    source_out << YAML::EndMap << YAML::EndSeq << YAML::EndMap;
    std::ofstream source_of((config.source_path.root_path() / BackupConfigManager::ConfigName).string());
    source_of << source_out.c_str();
    source_of.close();

    YAML::Emitter dest_out;
    dest_out << YAML::BeginMap << YAML::Key << "backups" << YAML::BeginSeq << YAML::BeginMap;
    dest_out << YAML::Key << "id" << YAML::Value << "A";
    dest_out << YAML::Key << "destination" << YAML::Value << fs::relative(config.destination_path, config.destination_path.root_path()).string();
//    dest_out << YAML::Key << "min_compact_size" << YAML::Value << config.min_compact_size;
//    dest_out << YAML::Key << "min_merge_size" << YAML::Value << config.min_merge_size;
//    dest_out << YAML::Key << "copy_buffer_size" << YAML::Value << config.buffer_size;
//    dest_out << YAML::Key << "enable_file_system_aware_compact" << YAML::Value << config.enable_fs_aware;
//    dest_out << YAML::Key << "enable_file_system_agnostic_compact" << YAML::Value << config.enable_fs_agnostic;
//    dest_out << YAML::Key << "enable_merge" << YAML::Value << config.enable_merge;
    dest_out << YAML::EndMap << YAML::EndSeq << YAML::EndMap;
    std::ofstream dest_of((config.destination_path.root_path() / BackupConfigManager::ConfigName).string());
    dest_of << dest_out.c_str();
    dest_of.close();
}
