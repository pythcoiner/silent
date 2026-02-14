#include "MainWindow.h"
#include "AccountWidget.h"
#include "AppController.h"
#include "screens/MenuTab.h"
#include <algorithm>
#include <common.h>
#include <qlogging.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qwidget.h>

MainWindow::MainWindow(QWidget *parent) : Window(parent) {
    setWindowTitle("Silent - Silent Payments Wallet");
    resize(1024, 768);
    initWindow();
}

auto MainWindow::initWindow() -> void {
    if (m_init)
        return;

    m_tab = new QTabWidget(this);
    m_menu_tab = new MenuTab(this);

    updateTabs();

    setCentralWidget(m_tab);

    // Connect tab close signal
    connect(m_tab, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested,
            qontrol::UNIQUE);

    m_tab->setTabsClosable(true);
    m_init = true;
}

auto MainWindow::accountExists(const QString &name) -> bool {
    if (m_tabs.isEmpty())
        return false;
    return std::ranges::any_of(m_tabs, [&name](const auto &tab) -> auto { return tab.first == name; });
}

auto MainWindow::insertAccount(AccountWidget *account, const QString &name) -> void {
    if (!accountExists(name)) {
        m_tabs.append(QPair<QString, AccountWidget *>(name, account));
        updateTabs();
    } else {
        qCritical() << "Tab for account" << name << "already exists!";
    }
}

auto MainWindow::removeAccount(const QString &name) -> void {
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

auto MainWindow::updateTabs() -> void {
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

auto MainWindow::closeEvent(QCloseEvent *event) -> void {
    // Stop all account controllers
    for (const auto &tab : m_tabs) {
        tab.second->controller()->stop();
    }
    event->accept();
    Window::closeEvent(event);
}

auto MainWindow::onTabCloseRequested(int index) -> void {
    if (index >= 0 && index < m_tabs.size()) {
        auto name = m_tabs.at(index).first;
        auto *controller = AppController::get();
        controller->removeAccount(name);
    }
}

MainWindow::~MainWindow() = default;
