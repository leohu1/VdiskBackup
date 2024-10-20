#include <QtQml/qqmlextensionplugin.h>

#include <QDir>
#include <QLoggingCategory>
#include <QNetworkProxy>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSslConfiguration>
#include <QtWidgets/QApplication>

#ifdef FLUENTUI_BUILD_STATIC_LIB
#  if (QT_VERSION > QT_VERSION_CHECK(6, 2, 0))
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#  endif
#  include <FluentUI.h>
#endif

#ifdef WIN32
#include "Log.h"
#include "SettingsHelper.h"
#include "TranslateHelper.h"
#include "app_dmp.h"
#endif

int main(int argc, char *argv[]) {
    const char *uri = "example";
#ifdef WIN32
    ::SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
#else
    qputenv("QT_QUICK_CONTROLS_STYLE", "Default");
#endif
#ifdef Q_OS_LINUX
    // fix bug UOSv20 does not print logs
    qputenv("QT_LOGGING_RULES", "");
    // fix bug UOSv20 v-sync does not work
    qputenv("QSG_RENDER_LOOP", "basic");
#endif
    QApplication::setOrganizationName("LeoHu");
    QApplication::setOrganizationDomain("https://github.com/leohu1");
    QApplication::setApplicationName("VdiskBackup");
    QApplication::setApplicationDisplayName("VdiskBackSystem");
    QApplication::setApplicationVersion("1.0");
    QApplication::setQuitOnLastWindowClosed(false);
    SettingsHelper::getInstance()->init(argv);
    Log::setup(argv, uri);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#  endif
#endif
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    TranslateHelper::getInstance()->init(&engine);
#ifdef FLUENTUI_BUILD_STATIC_LIB
    FluentUI::getInstance()->registerTypes(&engine);
#endif
    const QUrl url(QStringLiteral("qrc:/frontend/App.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);
    const int exec = QApplication::exec();
    if (exec == 931) {
        QProcess::startDetached(qApp->applicationFilePath(), qApp->arguments());
    }
    return exec;
}
