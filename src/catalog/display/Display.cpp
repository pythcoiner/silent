#include "catalog/display/Display.h"
#include "theme/Palette.h"
#include "theme/Theme.h"

namespace catalog {

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
        break;
    case DisplayRole::Address:
        useMono = true;
        break;
    case DisplayRole::Amount:
        useMono = true;
        align = Qt::AlignRight;
        break;
    case DisplayRole::Sats:
        useMono = true;
        align = Qt::AlignRight;
        break;
    case DisplayRole::Outpoint:
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
        "  border-radius: %3px;"
        "  padding: %4px %5px;"
        "  color: %6;"
        "}"
        "QLineEdit[class=\"display\"]:focus {"
        "  border: 1px solid %1;"
        "}")
        .arg(p.borderLight.name())    // %1
        .arg(p.bgSecondary.name())    // %2
        .arg(radius::INPUT)           // %3
        .arg(resolve(Padding::XS))    // %4
        .arg(resolve(Padding::S))     // %5
        .arg(p.textSecondary.name()); // %6
}

} // namespace catalog
