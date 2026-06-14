#pragma once

#include "theme/Palette.h"
#include <QFrame>

namespace catalog {

class Separator : public QFrame {
    Q_OBJECT

public:
    enum class Role { Horizontal, Vertical };

    explicit Separator(Role role = Role::Horizontal, QWidget *parent = nullptr);
    void setRole(Role role);
    [[nodiscard]] auto role() const -> Role;
    // Use the stronger `border` color (as the table header rule does) instead of
    // the default lighter divider color.
    void setStrong(bool strong);
    static auto qss(const Palette &p) -> QString;

private:
    void applyRole();

    Role m_role = Role::Horizontal;
    bool m_strong = false;
};

} // namespace catalog
