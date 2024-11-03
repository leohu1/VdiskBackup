#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <iostream>

#ifdef WIN32
#  include <process.h>
#else
#  include <unistd.h>
#endif

#ifndef QT_ENDL
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#    define QT_ENDL Qt::endl
#  else
#    define QT_ENDL endl
#  endif
#endif

static QString g_app = {};
static QString g_file_path = {};

QString Log::prettyProductInfoWrapper() {
    auto productName = QSysInfo::prettyProductName();
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#  if defined(Q_OS_MACOS)
    auto macosVersionFile =
        QString::fromUtf8("/System/Library/CoreServices/.SystemVersionPlatform.plist");
    auto fi = QFileInfo(macosVersionFile);
    if (fi.exists() && fi.isReadable()) {
        auto plistFile = QFile(macosVersionFile);
        plistFile.open(QIODevice::ReadOnly);
        while (!plistFile.atEnd()) {
            auto line = plistFile.readLine();
            if (line.contains("ProductUserVisibleVersion")) {
                auto nextLine = plistFile.readLine();
                if (nextLine.contains("<string>")) {
                    QRegularExpression re(QString::fromUtf8("\\s*<string>(.*)</string>"));
                    auto matches = re.match(QString::fromUtf8(nextLine));
                    if (matches.hasMatch()) {
                        productName = QString::fromUtf8("macOS ") + matches.captured(1);
                        break;
                    }
                }
            }
        }
    }
#  endif
#endif
#if defined(Q_OS_WIN)
    QSettings regKey{
        QString::fromUtf8(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion)"),
        QSettings::NativeFormat};
    if (regKey.contains(QString::fromUtf8("CurrentBuildNumber"))) {
        auto buildNumber = regKey.value(QString::fromUtf8("CurrentBuildNumber")).toInt();
        if (buildNumber > 0) {
            if (buildNumber < 9200) {
                productName = QString::fromUtf8("Windows 7 build %1").arg(buildNumber);
            } else if (buildNumber < 10240) {
                productName = QString::fromUtf8("Windows 8 build %1").arg(buildNumber);
            } else if (buildNumber < 22000) {
                productName = QString::fromUtf8("Windows 10 build %1").arg(buildNumber);
            } else {
                productName = QString::fromUtf8("Windows 11 build %1").arg(buildNumber);
            }
        }
    }
#endif
    return productName;
}

static inline void messageHandler(const QtMsgType type, const QMessageLogContext &context,
                                  const QString &msg) {
    auto logger = spdlog::get(std::string(g_app.toLocal8Bit()));
    QByteArray localMsg = msg.toLocal8Bit();
    spdlog::source_loc source = {"unknown", 0, "unknown"};
    if (context.file){
        source.filename = context.file != nullptr ? context.file : "";
        source.line = context.line;
        source.funcname = context.function != nullptr ? context.function : "";
    }
    switch (type) {
        case QtDebugMsg:
            logger->log(source, spdlog::level::debug, std::string(msg.toLocal8Bit()));
            break;
        case QtInfoMsg:
            logger->log(source, spdlog::level::info, std::string(msg.toLocal8Bit()));
            break;
        case QtWarningMsg:
            logger->log(source, spdlog::level::warn, std::string(msg.toLocal8Bit()));
            break;
        case QtCriticalMsg:
            logger->log(source, spdlog::level::critical, std::string(msg.toLocal8Bit()));
            break;
        case QtFatalMsg:
            logger->log(source, spdlog::level::err, std::string(msg.toLocal8Bit()));
            std::abort();
    }
}

void Log::setup(char *argv[], const QString &app) {
    Q_ASSERT(!app.isEmpty());
    if (app.isEmpty()) {
        return;
    }
    static bool once = false;
    if (once) {
        return;
    }

    QString applicationPath = QString::fromStdString(argv[0]);
    once = true;
    g_app = app;
    const QString logFileName =
        QString("%1.log").arg(g_app);
    const QString logDirPath =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/log";
    const QDir logDir(logDirPath);
    if (!logDir.exists()) {
        logDir.mkpath(logDirPath);
    }
    g_file_path = logDir.filePath(logFileName);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string(logFileName.toLocal8Bit()), 1048576 * 5, 3);
    std::vector<spdlog::sink_ptr> sinks {console_sink, rotating_file_sink};
    auto logger = std::make_shared<spdlog::logger>(std::string(g_app.toLocal8Bit()),sinks.begin(), sinks.end());
    logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
    qInstallMessageHandler(messageHandler);
    qInfo() << "===================================================";
    qInfo() << "[AppName]" << g_app;
    qInfo() << "[AppPath]" << applicationPath;
    qInfo() << "[QtVersion]" << QT_VERSION_STR;
#ifdef WIN32
    qInfo() << "[ProcessId]" << QString::number(_getpid());
#else
    qInfo() << "[ProcessId]" << QString::number(getpid());
#endif
    qInfo() << "[DeviceInfo]";
    qInfo() << "  [DeviceId]" << QSysInfo::machineUniqueId();
    qInfo() << "  [Manufacturer]" << prettyProductInfoWrapper();
    qInfo() << "  [CPU_ABI]" << QSysInfo::currentCpuArchitecture();
    qInfo() << "[LOG_PATH]" << g_file_path;
    qInfo() << "===================================================";
}
