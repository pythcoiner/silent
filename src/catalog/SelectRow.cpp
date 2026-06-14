#include "catalog/SelectRow.h"

#include "theme/Icon.h"
#include "catalog/display/Label.h"
#include "theme/Palette.h"

#include <QEvent>
#include <Qontrol>

namespace catalog {

SelectRow::SelectRow(QWidget *parent) : QWidget(parent) {
    setProperty("class", "select-row");
    installEventFilter(this);

    m_icon = new Label(LabelRole::Body, this);
    m_icon->setFixedWidth(metric::SELECT_ROW_ICON_SIZE);
    m_title = new Label(LabelRole::Body, this);
    auto titleFont = m_title->font();
    titleFont.setWeight(QFont::Medium); // design title is weight 500
    m_title->setFont(titleFont);
    m_metadata = new Label(LabelRole::Mono, this);
    m_chevron = new Label(LabelRole::Body, this);
    m_chevron->setPixmap(icon::chevronRight(IconColor::Muted).pixmap(16, 16));

    (new qontrol::Row)
        ->margins(metric::SELECT_ROW_PADDING_H, metric::SELECT_ROW_PADDING_V,
                  metric::SELECT_ROW_PADDING_H, metric::SELECT_ROW_PADDING_V)
        ->spacing(resolve(Spacing::M))
        ->push(m_icon)
        ->push(m_title, 1)
        ->push(m_metadata)
        ->push(m_chevron)
        ->into(this);
}

void SelectRow::setIcon(const QIcon &icon) {
    m_icon->setPixmap(icon.pixmap(metric::SELECT_ROW_ICON_SIZE, metric::SELECT_ROW_ICON_SIZE));
}

void SelectRow::setTitle(const QString &title) {
    m_title->setText(title);
}

void SelectRow::setMetadata(const QString &metadata) {
    m_metadata->setText(metadata);
}

auto SelectRow::eventFilter(QObject *watched, QEvent *event) -> bool {
    if (watched == this && event->type() == QEvent::MouseButtonRelease) {
        emit clicked();
    }
    return QWidget::eventFilter(watched, event);
}

auto SelectRow::qss(const Palette &p) -> QString {
    return QString("QWidget[class=\"select-row\"] { background: %1; border: 1px solid %2; "
                   "border-radius: %3px; }"
                   "QWidget[class=\"select-row\"]:hover { background: %4; border-color: %5; }"
                   "QWidget[class=\"select-row\"] QLabel[role=\"mono\"] { color: %6; }")
        .arg(p.surface.name())
        .arg(p.borderLight.name())
        .arg(radius::BUTTON)
        .arg(p.surfaceHover.name())
        .arg(p.border.name())
        .arg(p.textMuted.name());
}

} // namespace catalog
