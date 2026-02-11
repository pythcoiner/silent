#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <Qontrol>
#include <QPushButton>

class MenuTab : public QWidget {
    Q_OBJECT

public:
    explicit MenuTab(QWidget *parent = nullptr);

signals:
    void createAccount();

public slots:
    void onAccountList(const QList<QString> &accounts);

private:
    void initUI();
    void clearAccountButtons();

    QVBoxLayout *m_accountListLayout = nullptr;
    QList<QWidget *> m_accountRows;
};
