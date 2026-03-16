#include "Display.h"
#include "Palette.h"
#include "Theme.h"

namespace theme {

Display::Display(const QString &text, DisplayRole role, QWidget *parent)
    : QLineEdit(text, parent),
      m_role(role) {
    setReadOnly(true);
    setProperty("class", "display");
    applyRole();
}

Display::Display(DisplayRole role, QWidget *parent)
    : QLineEdit(parent),
      m_role(role) {
    setReadOnly(true);
    setProperty("class", "display");
    applyRole();
}

void Display::setRole(DisplayRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto Display::role() const -> DisplayRole {
    return m_role;
}

void Display::setWidth(Size s) {
    setFixedWidth(resolve(s));
}

void Display::applyRole() {
    const auto &ip = Theme::get()->inputPalette();

    bool useMono = false;
    Qt::Alignment align = Qt::AlignLeft;

    switch (m_role) {
    case DisplayRole::Default:
        setProperty("role", "default");
        break;
    case DisplayRole::Address:
        setProperty("role", "address");
        useMono = true;
        break;
    case DisplayRole::Amount:
        setProperty("role", "amount");
        align = Qt::AlignRight;
        break;
    case DisplayRole::Sats:
        setProperty("role", "sats");
        useMono = true;
        align = Qt::AlignRight;
        break;
    case DisplayRole::Outpoint:
        setProperty("role", "outpoint");
        useMono = true;
        break;
    }

    const auto &style = useMono ? ip.mono : ip.display;
    auto f = font();
    f.setPointSize(style.font.size);
    f.setFamily(style.font.family);
    f.setWeight(style.font.weight);
    setFont(f);
    setAlignment(align);
}

auto Display::qss(const Palette &p) -> QString {
    return QString(
        "QLineEdit[class=\"display\"] {"
        "  border: 1px solid %1;"
        "  background: %2;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "  color: %3;"
        "}"
        "QLineEdit[class=\"display\"]:focus {"
        "  border: 1px solid %1;"
        "}")
        .arg(p.borderLight.name())    // %1
        .arg(p.bgSecondary.name())    // %2
        .arg(p.textSecondary.name()); // %3
}

} // namespace theme
