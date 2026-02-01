#pragma once

#include <QWidget>
#include <Qontrol>
#include <QPushButton>

class MenuTab : public QWidget {
    Q_OBJECT

public:
    explicit MenuTab(QWidget *parent = nullptr);

signals:
    void createAccount();

private:
    void initUI();
};
