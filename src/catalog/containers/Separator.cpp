#include "catalog/containers/Separator.h"

#include <QSizePolicy>
#include <QStyle>

namespace catalog {

Separator::Separator(Role role, QWidget *parent) : QFrame(parent), m_role(role) {
    setProperty("class", "separator");
    setProperty("strong", false);
    setFrameShape(QFrame::NoFrame);
    applyRole();
}

void Separator::setStrong(bool strong) {
    if (m_strong == strong) {
        return;
    }
    m_strong = strong;
    setProperty("strong", strong);
    style()->unpolish(this);
    style()->polish(this);
}

void Separator::setRole(Role role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto Separator::role() const -> Role {
    return m_role;
}

void Separator::applyRole() {
    switch (m_role) {
    case Role::Horizontal:
        setProperty("role", "horizontal");
        setFixedHeight(1);
        setMinimumWidth(0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        break;
    case Role::Vertical:
        setProperty("role", "vertical");
        setFixedWidth(1);
        setMinimumHeight(0);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        break;
    }
}

auto Separator::qss(const Palette &p) -> QString {
    return QString("QFrame[class=\"separator\"] { border: none; background: %1; }"
                   "QFrame[class=\"separator\"][strong=\"true\"] { background: %2; }")
        .arg(p.borderLight.name())
        .arg(p.border.name());
}

} // namespace catalog
