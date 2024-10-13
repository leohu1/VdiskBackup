#include "BackupConfigManager.h"
#include "VirtDiskSystem.h"
#include "clipp.h"
#include "spdlog/spdlog.h"
#include <csignal>
#include <iostream>
using namespace clipp;
using namespace std;


int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, ".utf-8");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    signal(SIGINT, [](int i){
        std::cout << "Interrupt";
        exit(i);
    });
    //spdlog::init_thread_pool(8192, 1);
    spdlog::flush_every(std::chrono::seconds(5));

    enum class mode {backup, config, help};
    mode selected = mode::backup;

    auto makeMode = (command("backup").set(selected,mode::backup));

    auto findMode = (command("config").set(selected,mode::config));

    auto cli = (
            (makeMode | findMode | command("help").set(selected,mode::help) ),
            clipp::option("-v", "--version").call([]{cout << "version 1.0\n\n";}).doc("show version")  );

    if(parse(argc, argv, cli)) {
        switch(selected) {
            case mode::config: {
                BackupConfigManager config_manager;
                config_manager.ShowGUI();
                break;
            }
            case mode::backup: {
                VdiskBackupManager backup_manager;
                backup_manager.Init();
                backup_manager.GetAllConfigs();
                backup_manager.StartBackup();
                backup_manager.CleanUp();
                break;
            }
            case mode::help: cout << make_man_page(cli, "Vdiskbakup"); break;
        }
    } else {
        cout << usage_lines(cli, "Vdiskbackup") << '\n';
    }

    spdlog::drop_all();
    return 0;
}
