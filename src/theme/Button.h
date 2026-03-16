#pragma once

#include <QPushButton>
#include <QString>

struct Palette;

namespace theme {

enum class ButtonRole { Default, Primary, Destructive, Menu, Icon, InlineIcon, TabOpen, TabCreate, Inline };

class Button : public QPushButton {
    Q_OBJECT

public:
    explicit Button(const QString &text, ButtonRole role = ButtonRole::Default,
                    QWidget *parent = nullptr);
    explicit Button(ButtonRole role = ButtonRole::Default, QWidget *parent = nullptr);
    void setRole(ButtonRole role);
    [[nodiscard]] auto role() const -> ButtonRole;
    static auto qss(const Palette &p) -> QString;

private:
    ButtonRole m_role = ButtonRole::Default;
    void applyRole();
};

} // namespace theme
