#pragma once

#include <QWidget>

namespace theme {
class Button;
}

#include <Qontrol>

class MenuTab : public QWidget {
    Q_OBJECT

public:
    explicit MenuTab(QWidget *parent = nullptr);

signals:
    void createAccount();

public slots:
    void onAccountList(const QList<QString> &accounts);
    void onCreateClicked();
    void onAccountClicked();
    void onTrashClicked();

protected:
    void init();
    void doConnect();
    void view();
    void clearAccountButtons();

private:
    theme::Button *m_create_btn = nullptr;
    qontrol::Column *m_accounts_column = nullptr;
    QList<QWidget *> m_account_rows;
};
