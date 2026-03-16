#pragma once

#include <QCloseEvent>
#include <QList>
#include <QPair>
#include <QTabWidget>
#include <QWidget>
#include <Qontrol>

class AccountWidget;

class MainWindow : public qontrol::Window {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    auto insertAccount(AccountWidget *account, const QString &name) -> void;
    auto removeAccount(const QString &name) -> void;
    auto accountExists(const QString &name) -> bool;
    auto updateTabs() -> void;

public slots:
    auto onTabCloseRequested(int index) -> void;

protected:
    auto closeEvent(QCloseEvent *event) -> void override;

protected:
    auto initWindow() -> void;

private:
    bool m_init = false;
    QTabWidget *m_tab = nullptr;
    QList<QPair<QString, AccountWidget *>> m_tabs;
    QWidget *m_menu_tab = nullptr;
};
