#include <iostream>
#include "common_util/common_util.h"
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include "VolumeSystem.h"
#include "VdiskBackupManager.h"

using namespace log4cplus;

static std::mutex g_log_mtx_;

void static library_log_func(cutl::loglevel level, const std::string &msg)
{
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("cutl"));
    std::lock_guard<std::mutex> lock(g_log_mtx_);
    auto curTime = cutl::fmt_timestamp_ms(cutl::timestamp(cutl::timeunit::ms));
    auto threadId = std::this_thread::get_id();
    switch (level) {
        case cutl::loglevel::debug_level:
            LOG4CPLUS_DEBUG(logger, msg.c_str());
            break;
        case cutl::loglevel::info_level:
            LOG4CPLUS_INFO(logger, msg.c_str());
            break;
        case cutl::loglevel::warn_level:
            LOG4CPLUS_WARN(logger, msg.c_str());
            break;
        case cutl::loglevel::error_level:
            LOG4CPLUS_ERROR(logger, msg.c_str());
            break;
    }
}

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
      DWORD dwOriginalOutMode = 0;
      GetConsoleMode(hOut, &dwOriginalOutMode);

      // Enable ANSI escape code processing
      DWORD dwRequestedOutModes = dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      if (!SetConsoleMode(hOut, dwRequestedOutModes)) {
        SetConsoleMode(hOut, dwOriginalOutMode);
      }
    }
    PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("./log.conf"));
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));
    cutl::library_init(library_log_func);

    VdiskBackupManager manager;
    manager.GetAllConfigs();
    manager.GetCopySettings();
    manager.DoCopy();
    log4cplus::deinitialize();
    return 0;
}
