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

    connect(m_tab, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested,
            qontrol::UNIQUE);

    m_tab->setTabsClosable(true);
    m_tab->setMovable(true);
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
    // Find the account in m_tabs by name
    int listIndex = -1;
    AccountWidget *widget = nullptr;
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs.at(i).first == name) {
            listIndex = i;
            widget = m_tabs.at(i).second;
            break;
        }
    }

    if (listIndex < 0 || widget == nullptr) {
        return;
    }

    // Find the visual tab index by widget
    int tabIndex = m_tab->indexOf(widget);
    if (tabIndex >= 0) {
        m_tab->removeTab(tabIndex);
    }

    // Remove from list and delete widget
    m_tabs.removeAt(listIndex);
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
    auto *widget = qobject_cast<AccountWidget *>(m_tab->widget(index));
    if (widget == nullptr) {
        return; // Menu tab or invalid
    }

    // Find account name by widget
    for (const auto &tab : m_tabs) {
        if (tab.second == widget) {
            AppController::get()->removeAccount(tab.first);
            return;
        }
    }
}

MainWindow::~MainWindow() = default;
