#include "catalog/inputs/Checkbox.h"
#include "theme/Palette.h"
#include "theme/Theme.h"
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOption>

namespace catalog {

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
    if (!isEnabled()) {
        painter.setOpacity(0.55); // design dims the whole control when disabled
    }

    const auto &p = Theme::get()->palette();
    constexpr int c_margin = 1; // design box is flush; 1px keeps the 1.5px stroke unclipped

    // Box position (vertically centered)
    int boxY = (height() - metric::CHECKBOX_BOX) / 2;
    QRect boxRect(c_margin, boxY, metric::CHECKBOX_BOX, metric::CHECKBOX_BOX);

    // Draw box background + border
    painter.setPen(QPen(isChecked() ? p.accent : p.inputBorder, 1.5));
    painter.setBrush(isChecked() ? p.accent : p.inputBg);
    painter.drawRoundedRect(boxRect, radius::CHECKBOX, radius::CHECKBOX);

    // Draw checkmark when checked
    if (isChecked()) {
        painter.setPen(QPen(p.onAccent, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        int x = boxRect.x() + 4;
        int y = boxRect.y() + 4;
        int w = metric::CHECKBOX_BOX - 8;
        int h = metric::CHECKBOX_BOX - 8;

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
        painter.drawRoundedRect(boxRect, radius::CHECKBOX, radius::CHECKBOX);
    }

    // Draw text label if present
    QString label = text();
    if (!label.isEmpty()) {
        painter.setPen(p.text);
        int textX = c_margin + metric::CHECKBOX_BOX + 6;
        QRect textRect(textX, 0, width() - textX, height());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    }
}

auto Checkbox::sizeHint() const -> QSize {
    constexpr int c_margin = 1;
    int w = c_margin + metric::CHECKBOX_BOX + c_margin;
    int h = metric::CHECKBOX_BOX + 2 * c_margin;

    QString label = text();
    if (!label.isEmpty()) {
        QFontMetrics fm(font());
        w += 6 + fm.horizontalAdvance(label);
        h = qMax(h, fm.height() + 2 * c_margin);
    }

    return {w, h};
}

auto Checkbox::hitButton(const QPoint &pos) const -> bool {
    return rect().contains(pos);
}

auto Checkbox::qss([[maybe_unused]] const Palette &p) -> QString {
    // Paint-based: QSS not used for rendering
    return {};
}

} // namespace catalog
