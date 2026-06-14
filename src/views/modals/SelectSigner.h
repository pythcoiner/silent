#pragma once

#include <Qontrol>

namespace catalog {
class Button;
}

namespace modal {

class SelectSigner : public qontrol::Modal {
    Q_OBJECT

public:
    explicit SelectSigner(const QString &title, bool include_sd = true, QWidget *parent = nullptr);
    [[nodiscard]] auto selectedName() const -> QString;
    [[nodiscard]] auto selectedKind() const -> QString;

public slots:
    void onSignerClicked();

protected:
    void init();
    void doConnect();
    void view();

private:
    QString m_title;
    QString m_selected_name;
    QString m_selected_kind;
    bool m_include_sd = true;
    catalog::Button *m_cancel_btn = nullptr;
};

} // namespace modal
