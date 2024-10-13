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

    enum class mode {make, find, help};
    mode selected = mode::help;
    std::vector<std::string> input;
    std::string dict, out;
    bool split = false, progr = false;

    auto dictionary = required("-dict") & value("dictionary", dict);

    auto makeMode = (
            command("make").set(selected,mode::make),
            values("wordfile", input),
            dictionary,
            option("--progress", "-p").set(progr) % "show progress" );

    auto findMode = (
            command("find").set(selected,mode::find),
            values("infile", input),
            dictionary,
            (option("-o", "--output") & value("outfile", out)) % "write to file instead of stdout",
            ( option("-split"  ).set(split,true) |
             option("-nosplit").set(split,false) ) % "(do not) split output" );

    auto cli = (
            (makeMode | findMode | command("help").set(selected,mode::help) ),
            option("-v", "--version").call([]{cout << "version 1.0\n\n";}).doc("show version")  );

    if(parse(argc, argv, cli)) {
        switch(selected) {
            case mode::make: /* ... */ break;
            case mode::find: /* ... */ break;
            case mode::help: cout << make_man_page(cli, "finder"); break;
        }
    } else {
        cout << usage_lines(cli, "finder") << '\n';
    }


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
