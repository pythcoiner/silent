#pragma once

#include <QString>
#include <Qontrol>
#include <qevent.h>

namespace theme {
class Button;
}

namespace theme {
class Label;
}

namespace modal {

class ConfirmDelete : public qontrol::Modal {
    Q_OBJECT

public:
    explicit ConfirmDelete(const QString &name, QWidget *parent = nullptr);

signals:
    void confirmed(const QString &name);

public slots:
    void onDeleteClicked();

protected:
    void init();
    void doConnect();
    void view();
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    QString m_name;
    theme::Label *m_label = nullptr;
    theme::Button *m_cancel_btn = nullptr;
    theme::Button *m_delete_btn = nullptr;
};

} // namespace modal
