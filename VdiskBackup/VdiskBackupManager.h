//
// Created by 26071 on 2024/9/16.
//

#ifndef VDISKBACKUP_VDISKBACKUPMANAGER_H
#define VDISKBACKUP_VDISKBACKUPMANAGER_H

#include "log4cplus/include/log4cplus/logger.h"
#include "log4cplus/include/log4cplus/loggingmacros.h"
#include "log4cplus/include/log4cplus/configurator.h"
#include "common_util/filepath.h"
#include <string>
#include <utility>
#include <vector>

using namespace log4cplus;

enum VdiskBackupConfigType{
    From,
    To
};

struct VdiskBackupConfig{
public:
    VdiskBackupConfig(VdiskBackupConfigType t, const cutl::filepath& p, std::string i, const cutl::filepath& c):
    type(t), path(p), id(std::move(i)), config_path(c){}
    VdiskBackupConfigType type;
    cutl::filepath path;
    std::string id;
    cutl::filepath config_path;

};

struct VdiskCopySetting{
public:
    VdiskCopySetting(const cutl::filepath& fp, const cutl::filepath& tp): from_path(fp), to_path(tp){}
    cutl::filepath from_path;
    cutl::filepath to_path;
};

class VdiskBackupManager {
const std::string ConfigName = "VdiskBackupConfig.yaml";
private:
    std::multimap<std::string, VdiskBackupConfig> configs;
    std::vector<VdiskCopySetting> copy_settings;
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("VdiskBackupManager"));
public:
    VdiskBackupManager();
    void GetAllConfigs();
    void GetCopySettings();
    void DoCopy();
};


#endif //VDISKBACKUP_VDISKBACKUPMANAGER_H