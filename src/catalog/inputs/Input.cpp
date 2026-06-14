#include "catalog/inputs/Input.h"
#include "theme/Palette.h"
#include "theme/Theme.h"

namespace catalog {

Input::Input(const QString &text, InputRole role, QWidget *parent)
    : QLineEdit(text, parent),
      m_role(role) {
    setProperty("class", "input");
    applyRole();
}

Input::Input(InputRole role, QWidget *parent) : QLineEdit(parent), m_role(role) {
    setProperty("class", "input");
    applyRole();
}

void Input::setRole(InputRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto Input::role() const -> InputRole {
    return m_role;
}

void Input::setWidth(Size s) {
    setFixedWidth(resolve(s));
}

void Input::applyRole() {
    const auto &ip = Theme::get()->inputPalette();

    const auto *style = &ip.defaultInput;
    switch (m_role) {
    case InputRole::Default:
        style = &ip.defaultInput;
        setProperty("role", "default");
        break;
    case InputRole::Mono:
        style = &ip.mono;
        setProperty("role", "mono");
        break;
    case InputRole::Table:
        style = &ip.defaultInput;
        setProperty("role", "table");
        break;
    }

    auto f = font();
    f.setPointSize(style->font.size);
    f.setFamily(style->font.family);
    f.setWeight(style->font.weight);
    setFont(f);
}

auto Input::qss(const Palette &p) -> QString {
    return QString(
        "QLineEdit[class=\"input\"] {"
        "  border: 1px solid %1;"
        "  background: %2;"
        "  border-radius: %3px;"
        "  padding: %4px %5px;"
        "  color: %6;"
        "}"
        "QLineEdit[class=\"input\"]:focus {"
        "  border: 1px solid %7;"
        "}"
        "QLineEdit[class=\"input\"]:disabled {"
        "  background: %8;"
        "  color: %9;"
        "  border-color: %10;"
        "}")
        .arg(p.inputBorder.name()) // %1
        .arg(p.inputBg.name())     // %2
        .arg(radius::INPUT)        // %3
        .arg(resolve(Padding::XS)) // %4
        .arg(resolve(Padding::S))  // %5
        .arg(p.text.name())        // %6
        .arg(p.inputFocus.name())  // %7
        .arg(p.bgSecondary.name()) // %8
        .arg(p.textSecondary.name()) // %9
        .arg(p.borderLight.name())   // %10
        // In-cell table editor: borderless, no padding so the text stays put. It
        // takes the opposite row's stripe color (odd rows get the stripe color,
        // even rows get the base color) so the edited cell stands out in place.
        + QString("QLineEdit[class=\"input\"][role=\"table\"] {"
                  "  border: none;"
                  "  background: %1;"
                  "  border-radius: 0;"
                  "  padding: 0;"
                  "}"
                  "QLineEdit[class=\"input\"][role=\"table\"][onAlt=\"true\"] { background: %2; }"
                  "QLineEdit[class=\"input\"][role=\"table\"]:focus { border: none; }")
              .arg(p.bgSecondary.name()) // %1 odd rows -> stripe color
              .arg(p.bg.name());         // %2 even rows -> base color
}

} // namespace catalog
