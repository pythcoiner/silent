#include "catalog/StatusBarItem.h"

#include "catalog/display/Label.h"
#include "theme/Palette.h"
#include "catalog/inputs/Toggle.h"

#include <Qontrol>
#include <common.h>

namespace catalog {

StatusBarItem::StatusBarItem(QWidget *parent) : QWidget(parent) {
    m_toggle = new Toggle(ToggleRole::Status, this);
    m_label = new Label(LabelRole::Status, this);

    (new qontrol::Row)->spacing(resolve(Spacing::XS))->push(m_toggle)->push(m_label)->into(this);

    connect(m_toggle, &QCheckBox::toggled, this, &StatusBarItem::toggled, qontrol::UNIQUE);
}

void StatusBarItem::setLabel(const QString &label) {
    m_label->setText(label);
}

void StatusBarItem::setOn(bool on) {
    m_toggle->setChecked(on);
}

} // namespace catalog
