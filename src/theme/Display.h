#pragma once

#include "Palette.h"
#include <QLineEdit>

namespace theme {

enum class DisplayRole { Default, Address, Amount, Sats, Outpoint };

class Display : public QLineEdit {
    Q_OBJECT

public:
    explicit Display(const QString &text, DisplayRole role = DisplayRole::Default,
                     QWidget *parent = nullptr);
    explicit Display(DisplayRole role = DisplayRole::Default, QWidget *parent = nullptr);
    void setRole(DisplayRole role);
    [[nodiscard]] auto role() const -> DisplayRole;
    void setWidth(Size s);
    static auto qss(const Palette &p) -> QString;

private:
    DisplayRole m_role = DisplayRole::Default;
    void applyRole();
};

} // namespace theme
