#include "VdiskBackupManager.h"
#include "VirtDiskSystem.h"
#include "common_util/common_util.h"
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

static std::mutex g_log_mtx_;

void static library_log_func(cutl::loglevel level, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(g_log_mtx_);
    auto curTime = cutl::fmt_timestamp_ms(cutl::timestamp(cutl::timeunit::ms));
    auto threadId = std::this_thread::get_id();
    switch (level) {
        case cutl::loglevel::debug_level:
            SPDLOG_DEBUG(msg);
            break;
        case cutl::loglevel::info_level:
            SPDLOG_INFO(msg);
            break;
        case cutl::loglevel::warn_level:
            SPDLOG_WARN(msg);
            break;
        case cutl::loglevel::error_level:
            SPDLOG_ERROR(msg);
            break;
    }
}

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
    cutl::library_init(library_log_func);

    VdiskBackupManager manager;
    manager.GetAllConfigs();
    manager.StartBackup();

    spdlog::drop_all();
    return 0;
}
