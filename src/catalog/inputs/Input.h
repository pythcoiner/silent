#pragma once

#include "theme/Palette.h"
#include <QLineEdit>

namespace catalog {

enum class InputRole { Default, Mono, Table };

class Input : public QLineEdit {
    Q_OBJECT

public:
    explicit Input(const QString &text, InputRole role = InputRole::Default,
                   QWidget *parent = nullptr);
    explicit Input(InputRole role = InputRole::Default, QWidget *parent = nullptr);
    void setRole(InputRole role);
    [[nodiscard]] auto role() const -> InputRole;
    void setWidth(Size s);
    static auto qss(const Palette &p) -> QString;

private:
    InputRole m_role = InputRole::Default;
    void applyRole();
};

} // namespace catalog
