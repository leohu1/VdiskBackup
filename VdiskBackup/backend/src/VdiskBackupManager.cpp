//
// Created by 26071 on 2024/9/16.
//

#include "VdiskBackupManager.h"
#include "BackupConfigManager.h"
#include "FileSystem.h"
#include "VirtDiskSystem.h"
#include "VolumeSystem.h"
#include "spdlog/spdlog.h"
#include "utils.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include <iostream>

VdiskBackupManager::VdiskBackupManager() {
    SPDLOG_INFO("VdiskBackupManager initialized");
}

void VdiskBackupManager::GetAllConfigs() {
    // 获取全部拷贝文件，存储为 VdiskBackupConfig
    std::vector<VolumeInfo> vec = VolumeSystem::GetAllVolumeInfo();

    for (auto & it : vec) {
        if (!it.drive_letter.empty()){
            fs::path base_path = utils::removeSpaces(it.drive_letter) + "\\";
            fs::path path = base_path / BackupConfigManager::ConfigName;
            if (fs::exists(path)){
                SPDLOG_INFO("Get config at " + fs::absolute(path).string());
                YAML::Node configs_data = YAML::LoadFile(fs::absolute(path).string());
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
                            config.id = id;
                            config.config_paths.push_back(path);
                            if (i["source"]){
                                config.source_path = fs::absolute(base_path / i["source"].as<string>());
                            }
                            if (i["destination"]){
                                config.destination_path = fs::absolute(base_path / i["destination"].as<string>());
                            }
                            if (i["min_compact_size"]){
                                config.min_compact_size = i["min_compact_size"].as<size_t>() * 1024 * 1024 *1024;
                            }
                            if (i["min_merge_size"]){
                                config.min_merge_size = i["min_merge_size"].as<size_t>() * 1024 * 1024 *1024;
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
                            if (i["enable_merge"]){
                                config.enable_merge = i["enable_merge"].as<bool>();
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
        if (fs::exists(config.source_path)){
            std::map<fs::path, std::string> md5_map;
            fs::create_directories(config.destination_path);
            for (const auto& k : VdiskBackupManager::GetLastMd5(config.destination_path)){
                fs::path p = config.destination_path / k.first;
                if(fs::exists(p)){
                    md5_map.insert(make_pair(absolute(p), k.second));
                }
            }
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "id" << YAML::Value << i.first;
            out << YAML::Key << "result" << YAML::Value << YAML::BeginSeq;
            if (config.enable_fs_aware){
                FileSize original_file_size = FileSystem::GetFileSize(config.source_path.string());
                if (original_file_size.GetSizeBits() >= config.min_compact_size){
                    SPDLOG_INFO("Compact vhdx with filesystem aware, file: {}", config.source_path.string());
                    VirtDiskSystem::CompactVdisk(config.source_path.string(), VirtDiskSystem::FSAware);
                    out << YAML::BeginMap;
                    out << YAML::Key << "id" << YAML::Value << "fs_aware_compact";
                    out << YAML::Key << "original_size" << YAML::Value << original_file_size.GetSizeBits();
                    out << YAML::Key << "result_size" << YAML::Value << FileSystem::GetFileSize(config.source_path.string()).GetSizeBits();
                    out << YAML::EndMap;
                    if (md5_map.contains(config.source_path)){
                        md5_map.erase(config.source_path);
                    }
                }
            }
            if (config.enable_fs_agnostic){
                FileSize original_file_size = FileSystem::GetFileSize(config.source_path.string());
                if (original_file_size.GetSizeBits() >= config.min_compact_size){
                    SPDLOG_INFO("Compact vhdx with filesystem agnostic, file: {}", config.source_path.string());
                    VirtDiskSystem::CompactVdisk(config.source_path.string(), VirtDiskSystem::FSAgnostic);
                    out << YAML::BeginMap;
                    out << YAML::Key << "id" << YAML::Value << "fs_agnostic_compact";
                    out << YAML::Key << "original_size" << YAML::Value << original_file_size.GetSizeBits();
                    out << YAML::Key << "result_size" << YAML::Value << FileSystem::GetFileSize(config.source_path.string()).GetSizeBits();
                    out << YAML::EndMap;
                    if (md5_map.contains(config.source_path)){
                        md5_map.erase(config.source_path);
                    }
                }
            }
            if (config.enable_merge){
                FileSize original_file_size = FileSystem::GetFileSize(config.source_path.string());
                if (original_file_size.GetSizeBits() >= config.min_merge_size) {
                    fs::path parent_path = VirtDiskSystem::GetVdiskParent(config.source_path.string());
                    if (fs::exists(parent_path)) {
                        SPDLOG_INFO("Merging vhdx {} to {}", config.source_path.string(), fs::absolute(parent_path).string());
                        VirtDiskSystem::MergeVdiskToParent(config.source_path.string());
                        DWORD dw_error;
                        if (DeleteFileW(utils::charToLPWSTR(config.source_path.string().c_str())) == 0){
                            dw_error = GetLastError();
                            SPDLOG_ERROR("Get Error ({}): {}\n", dw_error, utils::ErrorMessage(dw_error));
                        }
                        fs::rename(parent_path, config.source_path);
                        out << YAML::BeginMap;
                        out << YAML::Key << "id" << YAML::Value << "merge_vdisk";
                        out << YAML::Key << "original_size" << YAML::Value << original_file_size.GetSizeBits();
                        out << YAML::Key << "result_size" << YAML::Value << FileSystem::GetFileSize(config.source_path.string()).GetSizeBits();
                        out << YAML::Key << "parent_file" << YAML::Value << parent_path.string();
                        out << YAML::Key << "child_file" << YAML::Value << config.source_path.string();
                        out << YAML::EndMap;
                        if (md5_map.contains(config.source_path)){
                            md5_map.erase(config.source_path);
                        }
                        if (md5_map.contains(parent_path)){
                            md5_map.erase(parent_path);
                        }
                    }
                }
            }
            // vector with path of file to copy
            std::vector<fs::path> copy_files;
            copy_files.emplace_back(config.source_path);
            for (fs::path j : VirtDiskSystem::GetVdiskParents(config.source_path.string())){
                copy_files.emplace_back(j);
            }
            for (const auto& j : std::map<fs::path, string>(md5_map)){
                std::vector<fs::path>::iterator it;
                if ((it = std::find_if(copy_files.begin(), copy_files.end(), [j](const fs::path& p){
                         return p.filename() == j.first;
                     })) != copy_files.end()) {
                    if (GetFileMd5(j.first) == j.second){
                        md5_map.erase(j.first);
                        copy_files.erase(it);
                    }
                }
            }
            out << YAML::BeginMap;
            out << YAML::Key << "id" << YAML::Value << "copy_files";
            out << YAML::Key << "files" << YAML::Value;
            out << YAML::BeginSeq;

            for (const auto& o : copy_files){
                fs::path dest_path = config.destination_path / o.filename();
                std::string *md5;
                if (!FileSystem::CopyFileWithProgressBar(o,
                                                        dest_path,
                                                        &md5,
                                                        std::format("Backup {} -> {}", o.string(), dest_path.string()),
                                                        config.buffer_size)){
                    SPDLOG_ERROR("backup {} failed", config.source_path.string());
                    continue;
                }
                out << YAML::BeginMap;
                out << YAML::Key << "path" << YAML::Value << fs::relative(dest_path, config.destination_path).string();
                out << YAML::Key << "md5" << YAML::Value << *md5;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            out << YAML::EndMap;
            out << YAML::EndSeq;
            out << YAML::EndMap;
            std::ofstream f_result((config.destination_path / BackupConfigManager::BackupResult).string());
            f_result << out.c_str();
            f_result.close();
        }
    }
}
std::map<std::string, std::string> VdiskBackupManager::GetLastMd5(fs::path path) {
    path = path / BackupConfigManager::BackupResult;
    std::map<std::string, std::string> out;
    if (fs::exists(path)){
        YAML::Node config = YAML::LoadFile(path.string());
        if (config["result"].IsSequence()){
            YAML::Node result = config["result"];
            for (YAML::const_iterator it=result.begin();it!=result.end();++it) {
                if (it->operator[]("id") && it->operator[]("id").as<string>() == "copy_files"){
                    if(it->operator[]("files").IsSequence()){
                        YAML::Node files = it->operator[]("files");
                        for (YAML::const_iterator jt=files.begin();jt!=files.end();++jt) {
                            if (it->operator[]("path") && it->operator[]("md5")) {
                                out.insert(
                                        std::pair<std::string, std::string>(
                                                it->operator[]("path").as<string>(),
                                                it->operator[]("md5").as<string>()));
                                SPDLOG_INFO("Get last time's MD5 {}: {}",
                                            it->operator[]("path").as<string>(),
                                                    it->operator[]("md5").as<string>());
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    return out;
}

void VdiskBackupManager::CleanUp() {
    SPDLOG_INFO("Cleaning up configs");
    std::vector<std::string> log_messages = ringbuffer_sink->last_formatted();
    for (const auto& i : backup_configs){
        for (const auto& j : i.second.config_paths){
            if (fs::exists(j)){
                fs::remove(j);
            }
        }
        std::ofstream log(i.second.destination_path / "log.txt", std::ios::app);
        for (const auto& k : log_messages){
            log << k << std::endl;
        }
        log.close();
    }
}
void VdiskBackupManager::Init() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024);
    std::vector<spdlog::sink_ptr> sinks {console_sink, ringbuffer_sink};
    auto logger = std::make_shared<spdlog::logger>("vdisk_backup",sinks.begin(), sinks.end());
    logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

}

VdiskBackupConfig::~VdiskBackupConfig() = default;
