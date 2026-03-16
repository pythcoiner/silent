#include "Input.h"
#include "Palette.h"
#include "Theme.h"

namespace theme {

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
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "  color: %3;"
        "}"
        "QLineEdit[class=\"input\"]:focus {"
        "  border: 1px solid %4;"
        "}"
        "QLineEdit[class=\"input\"]:disabled {"
        "  background: %5;"
        "  color: %6;"
        "  border-color: %7;"
        "}")
        .arg(p.inputBorder.name()) // %1
        .arg(p.inputBg.name())     // %2
        .arg(p.text.name())        // %3
        .arg(p.inputFocus.name())      // %4
        .arg(p.bgSecondary.name())     // %5
        .arg(p.textSecondary.name())   // %6
        .arg(p.borderLight.name());    // %7
}

} // namespace theme
