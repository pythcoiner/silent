#pragma once

#include <QWidget>

namespace catalog {

class FormRow : public QWidget {
    Q_OBJECT

public:
    explicit FormRow(const QString &label_text, QWidget *control, QWidget *parent = nullptr);
    [[nodiscard]] auto control() const -> QWidget *;

private:
    QWidget *m_control = nullptr;
};

} // namespace catalog
