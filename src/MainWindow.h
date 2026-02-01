#pragma once

#include "AccountWidget.h"
#include <Qontrol>
#include <QTabWidget>
#include <QHash>
#include <QList>
#include <QPair>
#include <QWidget>
#include <QCloseEvent>

class AccountWidget;

class MainWindow : public qontrol::Window {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void insertAccount(AccountWidget *account, const QString &name);
    void removeAccount(const QString &name);
    auto accountExists(const QString &name) -> bool;
    void updateTabs();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool m_init = false;
    QTabWidget *m_tab = nullptr;
    QList<QPair<QString, AccountWidget *>> m_tabs;
    QWidget *m_menu_tab = nullptr;

    void initWindow();
};
