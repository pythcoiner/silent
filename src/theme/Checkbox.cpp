#include "Checkbox.h"
#include "Palette.h"
#include "Theme.h"
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOption>

namespace theme {

Checkbox::Checkbox(const QString &text, CheckboxRole role, QWidget *parent)
    : QCheckBox(text, parent),
      m_role(role) {
    applyRole();
}

Checkbox::Checkbox(CheckboxRole role, QWidget *parent) : QCheckBox(parent), m_role(role) {
    applyRole();
}

void Checkbox::setRole(CheckboxRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto Checkbox::role() const -> CheckboxRole {
    return m_role;
}

void Checkbox::applyRole() {
    setProperty("class", "checkbox");

    switch (m_role) {
    case CheckboxRole::Default:
        setProperty("role", "default");
        break;
    }

    const auto &fp = Theme::get()->fontPalette();
    auto f = font();
    f.setPointSize(fp.checkboxLabel.size);
    f.setFamily(fp.checkboxLabel.family);
    f.setWeight(fp.checkboxLabel.weight);
    setFont(f);
}

void Checkbox::paintEvent([[maybe_unused]] QPaintEvent *e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const auto &p = Theme::get()->palette();
    constexpr int c_box_size = 18;
    constexpr int c_radius = 4;
    constexpr int c_margin = 2;

    // Box position (vertically centered)
    int boxY = (height() - c_box_size) / 2;
    QRect boxRect(c_margin, boxY, c_box_size, c_box_size);

    // Draw box background + border
    painter.setPen(QPen(isChecked() ? p.accent : p.inputBorder, 1.5));
    painter.setBrush(isChecked() ? p.accent : p.inputBg);
    painter.drawRoundedRect(boxRect, c_radius, c_radius);

    // Draw checkmark when checked
    if (isChecked()) {
        painter.setPen(QPen(color::WHITE, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        int x = boxRect.x() + 4;
        int y = boxRect.y() + 4;
        int w = c_box_size - 8;
        int h = c_box_size - 8;

        QPainterPath path;
        path.moveTo(x, y + h * 0.5);
        path.lineTo(x + w * 0.35, y + h);
        path.lineTo(x + w, y);
        painter.drawPath(path);
    }

    // Draw hover border
    if (underMouse() && !isChecked()) {
        painter.setPen(QPen(p.inputFocus, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(boxRect, c_radius, c_radius);
    }

    // Draw text label if present
    QString label = text();
    if (!label.isEmpty()) {
        painter.setPen(isEnabled() ? p.text : p.textMuted);
        int textX = c_margin + c_box_size + 6;
        QRect textRect(textX, 0, width() - textX, height());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    }
}

auto Checkbox::sizeHint() const -> QSize {
    constexpr int c_box_size = 18;
    constexpr int c_margin = 2;
    int w = c_margin + c_box_size + c_margin;
    int h = c_box_size + 2 * c_margin;

    QString label = text();
    if (!label.isEmpty()) {
        QFontMetrics fm(font());
        w += 6 + fm.horizontalAdvance(label);
        h = qMax(h, fm.height() + 2 * c_margin);
    }

    return {w, h};
}

auto Checkbox::qss([[maybe_unused]] const Palette &p) -> QString {
    // Paint-based — QSS not used for rendering
    return {};
}

} // namespace theme
