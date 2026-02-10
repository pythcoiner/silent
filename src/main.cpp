#include <QApplication>
#include "AppController.h"
#include "MainWindow.h"
#include <silent.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Initialize Rust logging (Debug level for verbose output)
    init_logging(LogLevel::Debug);

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

    return app.exec();
}
