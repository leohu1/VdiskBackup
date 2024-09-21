//
// Created by 26071 on 2024/9/16.
//

#include "VdiskBackupManager.h"
#include "VolumeSystem.h"
#include "common_util/filepath.h"
#include "yaml-cpp/yaml.h"
#include "FileSystem.h"

std::string removeSpaces(const std::string& str) {
    std::string result;
    std::copy_if(str.begin(), str.end(), std::back_inserter(result), [](char c) {
        return !std::isspace(c);
    });
    return result;
}

VdiskBackupManager::VdiskBackupManager() {
    LOG4CPLUS_INFO(logger, L"VdiskBackupManager initialized");
}

void VdiskBackupManager::GetAllConfigs() {
    // 获取全部拷贝文件，存储为 VdiskBackupConfig
    std::vector<VolumeInfo> vec = VolumeSystem::GetAllVolumeInfo();

    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (!it->drive_letter.empty()){
            cutl::filepath base_path = cutl::filepath(removeSpaces(it->drive_letter));
            cutl::filepath path = base_path.join(VdiskBackupManager::ConfigName);
            if (path.exists()){
                LOG4CPLUS_INFO(logger, ("get config at " + path.abspath()).c_str());
                YAML::Node config = YAML::LoadFile(path.abspath());
                YAML::Node from = config["from"];
                if (from && from.IsSequence()){
                    for (auto && i : from) {
                        if (i["id"] && i["path"]){
                            cutl::filepath from_path = base_path.join(i["path"].as<std::string>());
                            configs.insert(make_pair(
                                    i["id"].as<std::string>(),
                                            VdiskBackupConfig(From,
                                                              from_path,
                                                              i["id"].as<std::string>(),
                                                              path)));
                        }
                    }
                }

                YAML::Node to = config["to"];
                if (to && to.IsSequence()){
                    for (auto && i : to) {
                        if (i["id"] && i["path"]){
                            cutl::filepath to_path = base_path.join(i["path"].as<std::string>());
                            configs.insert(make_pair(
                                    i["id"].as<std::string>(),
                                    VdiskBackupConfig(To,
                                                      to_path,
                                                      i["id"].as<std::string>(),
                                                      path)));
                        }
                    }
                }
            }
        }
    }
}

void VdiskBackupManager::GetCopySettings() {
    for (auto it = configs.begin();it != configs.end();it = configs.upper_bound(it->first)){
        int from = 0, to = 0;
        cutl::filepath *fp = nullptr, *tp = nullptr;
        for (auto range = configs.equal_range(it->first);range.first != range.second;++range.first){
            // key: it->first, items: range.first->second
            switch (range.first -> second.type){
                case From:
                    fp = &(range.first -> second.path);
                    ++from;
                    break;
                case To:
                    tp = &(range.first -> second.path);
                    ++to;
                    break;
            }
        }
        if (from == 1 && to == 1 && fp != nullptr && tp != nullptr){
            copy_settings.emplace_back(*fp, *tp);
        } else {
            LOG4CPLUS_WARN(logger,
                           std::format("id: {} get {:d} from and {:d} to, will be ignored.",
                                       it->first, from, to).c_str());
            configs.erase(it->first);
        }
    }
}

void VdiskBackupManager::DoCopy() {
    size_t buf_size = 8*1024;
    cutl::filepath cp = cutl::filepath("./CopySettings.yaml");
    if (cp.exists()){
        YAML::Node config = YAML::LoadFile(cp.abspath());
        if (config["buffer_size"]){
            buf_size = config["buffer_size"].as<size_t >();
        }
    }
    for (const auto& i : copy_settings){
        std::string t = std::format("Copying {} -> {} ", i.from_path.str(), i.to_path.str());
        LOG4CPLUS_INFO(logger, ("Start "+t).c_str());
        FileSystem::CopyFileWithProgressBar(
                i.from_path, i.to_path,
                t, buf_size);
        LOG4CPLUS_INFO(logger, ("Finish "+t).c_str());
    }
}
