#pragma once

#include <Qontrol>
#include <QString>

class ConfirmDelete : public qontrol::Modal {
    Q_OBJECT

public:
    explicit ConfirmDelete(const QString &name, QWidget *parent = nullptr);

signals:
    void confirmed(const QString &name);

private:
    QString m_name;
    void initUI();
};
