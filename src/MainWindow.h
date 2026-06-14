#pragma once

#include <QCloseEvent>
#include <QEvent>
#include <QList>
#include <QPair>
#include <QTabWidget>
#include <QWidget>
#include <Qontrol>

namespace theme {
class Button;
}

class MainWindow : public qontrol::Window {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    auto addTab(QWidget *content, const QString &title) -> int;
    auto removeTab(QWidget *content) -> void;
    auto setTabTitle(QWidget *content, const QString &title) -> void;
    auto tabExists(QWidget *content) const -> bool;
    auto updateTabs() -> void;

signals:
    void hostedTabCloseRequested(QWidget *content);

public slots:
    auto onTabCloseRequested(int index) -> void;

protected:
    auto closeEvent(QCloseEvent *event) -> void override;
    auto changeEvent(QEvent *event) -> void override;

protected:
    auto initWindow() -> void;
    auto retranslateUi() -> void;

private:
    bool m_init = false;
    QTabWidget *m_tab = nullptr;
    theme::Button *m_settings_btn = nullptr;
    QList<QPair<QString, QWidget *>> m_tabs;
    QWidget *m_menu_tab = nullptr;
};
