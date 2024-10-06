#include "VdiskBackupManager.h"
#include "VirtDiskSystem.h"
#include "BackupConfigManager.h"
#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include <locale>


int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, ".utf-8");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    spdlog::init_thread_pool(8192, 1);
    spdlog::flush_every(std::chrono::seconds(5));

    cxxopts::Options options("VdiskBackup", "A program to backup vdisk");
    options.add_options()
            ("b,backup", "Backup Mode", cxxopts::value<bool>()->default_value("false"))
                    ("c,config", "Config Mode", cxxopts::value<bool>()->default_value("true"));
    auto result = options.parse(argc, argv);
    bool config = result["config"].as<bool>();
    if (config) {
        BackupConfigManager manager;
        manager.ShowGUI();
    }
    bool backup = result["backup"].as<bool>();
    if (backup) {
        VdiskBackupManager manager;
        manager.Init();
        manager.GetAllConfigs();
        manager.StartBackup();
        manager.CleanUp();
    }
    spdlog::drop_all();
    return 0;
}
