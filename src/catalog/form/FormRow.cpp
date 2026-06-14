#include "catalog/form/FormRow.h"

#include "catalog/display/Label.h"
#include "theme/Palette.h"

#include <Qontrol>

namespace catalog {

FormRow::FormRow(const QString &label_text, QWidget *control, QWidget *parent)
    : QWidget(parent),
      m_control(control) {
    setProperty("class", "form-row");

    auto *label = new Label(label_text, LabelRole::InputLabel, this);
    label->setFixedWidth(metric::FORM_LABEL_WIDTH);
    (new qontrol::Row)
        ->spacing(resolve(Spacing::XS))
        ->push(label)
        ->push(m_control, 1)
        ->into(this);
}

auto FormRow::control() const -> QWidget * {
    return m_control;
}

} // namespace catalog
