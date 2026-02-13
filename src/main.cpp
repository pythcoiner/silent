#include "AppController.h"
#include "MainWindow.h"
#include <QApplication>
#include <silent.h>

auto main(int argc, char *argv[]) -> int {
    QApplication app(argc, argv);

    // Initialize Rust logging (Debug level for verbose output)
    init_logging(LogLevel::Info);

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
