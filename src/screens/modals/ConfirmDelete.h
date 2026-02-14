#pragma once

#include <QString>
#include <Qontrol>
#include <qlabel.h>
#include <qpushbutton.h>

class ConfirmDelete : public qontrol::Modal {
    Q_OBJECT

public:
    explicit ConfirmDelete(const QString &name, QWidget *parent = nullptr);

signals:
    void confirmed(const QString &name);

public slots:
    auto onDeleteClicked() -> void;

private:
    auto init() -> void;
    auto doConnect() -> void;
    auto view() -> void;

    QString m_name;
    QLabel *m_label = nullptr;
    QPushButton *m_cancel_btn = nullptr;
    QPushButton *m_delete_btn = nullptr;
};
