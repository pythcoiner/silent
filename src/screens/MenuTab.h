#pragma once

#include <QPushButton>
#include <QWidget>
#include <Qontrol>

class MenuTab : public QWidget {
    Q_OBJECT

public:
    explicit MenuTab(QWidget *parent = nullptr);

signals:
    void createAccount();

public slots:
    auto onAccountList(const QList<QString> &accounts) -> void;

private:
    auto init() -> void;
    auto doConnect() -> void;
    auto view() -> void;
    auto clearAccountButtons() -> void;

    QPushButton *m_create_btn = nullptr;
    qontrol::Column *m_accounts_column = nullptr;
    QList<QWidget *> m_account_rows;
};
