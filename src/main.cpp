#include "AppController.h"
#include "MainWindow.h"
#include "metatypes.h"
#include <QApplication>
#include <QStyleFactory>

auto main(int argc, char *argv[]) -> int {
    QApplication app(argc, argv);
#ifdef Q_OS_LINUX
    app.setStyle(QStyleFactory::create("Fusion"));
#endif

    // Register CXX types for cross-thread signal-slot connections
    qRegisterMetaType<BackendInfo>();
    qRegisterMetaType<ConnectionResult>();
    qRegisterMetaType<TxResult>();
    qRegisterMetaType<Notification>();

    // Initialize Rust logging (Debug level for verbose output)
    init_logging(LogLevel::Error);

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
