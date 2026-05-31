#include "AppController.h"
#include "MainWindow.h"
#include "i18n/I18nManager.h"
#include "metatypes.h"
#include "resources/font/noto_sans.h"
#include "theme/Theme.h"
#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>

auto main(int argc, char *argv[]) -> int {
    QApplication app(argc, argv);
#ifdef Q_OS_LINUX
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#endif

    // Embed default font
    QFontDatabase::addApplicationFontFromData(QByteArray(
        reinterpret_cast<const char *>(embedded_font::NOTO_SANS), embedded_font::NOTO_SANS_SIZE));

    // Register CXX types for cross-thread signal-slot connections
    qRegisterMetaType<BackendInfo>();
    qRegisterMetaType<ConnectionResult>();
    qRegisterMetaType<TxResult>();
    qRegisterMetaType<Notification>();

    // Initialize Rust logging (Debug level for verbose output)
    init_logging(LogLevel::Error);

    // Initialize and apply theme
    Theme::init();
    Theme::get()->setMode(ThemeMode::Light);
    Theme::get()->apply();

    // Initialize i18n before creating UI widgets
    i18n::I18nManager::get()->init(&app);

    // Initialize application controller
    AppController::init();
    auto *controller = AppController::get();

    // Create and show main window
    auto *window = new MainWindow();
    controller->start(window);

    // Initialize state (load accounts)
    controller->initState();

    // Show window
    window->show();

    return QApplication::exec();
}
