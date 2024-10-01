#include "VdiskBackupManager.h"
#include "VirtDiskSystem.h"
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::init_thread_pool(8192, 1);
    // Create a file rotating logger with 5 MB size max and 3 rotated files
    auto max_size = 1048576 * 5;
    auto max_files = 3;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/vdisk_backup.txt", max_size, max_files);
    file_sink->set_level(spdlog::level::info);
    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::async_logger>("vdisk_backup",
                                                         sinks.begin(), sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
    logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");
    logger->set_level(spdlog::level::debug);
    spdlog::flush_every(std::chrono::seconds(5));
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    VdiskBackupManager manager;
    manager.GetAllConfigs();
    manager.StartBackup();

    spdlog::drop_all();
    return 0;
}
