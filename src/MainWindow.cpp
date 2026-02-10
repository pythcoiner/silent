#include "MainWindow.h"
#include "AppController.h"
#include "screens/MenuTab.h"
#include <qlogging.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qwidget.h>

MainWindow::MainWindow(QWidget *parent) : Window(parent) {
    setWindowTitle("Silent - Silent Payments Wallet");
    resize(1024, 768);
    initWindow();
}

void MainWindow::initWindow() {
    if (m_init)
        return;

    m_tab = new QTabWidget(this);
    m_menu_tab = new MenuTab(this);

    updateTabs();

    setCentralWidget(m_tab);

    // Connect tab close signal
    connect(m_tab, &QTabWidget::tabCloseRequested, this, [this](int index) {
        // Find account name by index
        if (index >= 0 && index < m_tabs.size()) {
            auto name = m_tabs.at(index).first;
            auto *controller = AppController::get();
            controller->removeAccount(name);
        }
    });

    m_tab->setTabsClosable(true);
    m_init = true;
}

auto MainWindow::accountExists(const QString &name) -> bool {
    if (m_tabs.isEmpty())
        return false;
    for (const auto &tab : m_tabs) {
        if (tab.first == name)
            return true;
    }
    return false;
}

void MainWindow::insertAccount(AccountWidget *account, const QString &name) {
    if (!accountExists(name)) {
        m_tabs.append(QPair<QString, AccountWidget *>(name, account));
        updateTabs();
    } else {
        qCritical() << "Tab for account" << name << "already exists!";
    }
}

void MainWindow::removeAccount(const QString &name) {
    auto exists = false;
    int index = 0;
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs.at(i).first == name) {
            index = i;
            exists = true;
            break;
        }
    }

    if (!exists)
        return;

    // Remove widget from tab widget
    m_tab->removeTab(index);

    // Delete the widget
    auto *widget = m_tabs.at(index).second;
    m_tabs.removeAt(index);
    widget->deleteLater();
}

void MainWindow::updateTabs() {
    // Clear all tabs except the menu tab
    while (m_tab->count() > 0) {
        m_tab->removeTab(0);
    }

    // Add account tabs
    for (const auto &tab : m_tabs) {
        m_tab->addTab(tab.second, tab.first);
    }

    // Add menu tab at the end
    m_tab->addTab(m_menu_tab, "+");

    // Make the last tab (menu tab) not closable
    m_tab->tabBar()->setTabButton(m_tab->count() - 1, QTabBar::RightSide, nullptr);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Stop all account controllers
    for (const auto &tab : m_tabs) {
        tab.second->controller()->stop();
    }
    event->accept();
    Window::closeEvent(event);
}

MainWindow::~MainWindow() = default;
