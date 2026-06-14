#include "catalog/form/LabelledInput.h"

#include "catalog/display/Label.h"
#include "catalog/form/ValidationMark.h"
#include "catalog/inputs/Input.h"
#include "theme/Palette.h"

#include <Qontrol>

namespace catalog {

LabelledInput::LabelledInput(const QString &label, Input *input, ValidationMark *mark,
                             QWidget *parent)
    : QWidget(parent) {
    setProperty("class", "labelled-input");

    // Caption gives the small font; the qss() rule below recolors it to
    // secondary (overriding caption's default muted color) via the labelKind tag.
    auto *labelWidget = new Label(label, LabelRole::Caption, this);
    labelWidget->setProperty("labelKind", "labelled");

    auto *inputRow = (new qontrol::Row)->spacing(resolve(Spacing::XS))->push(input, 1);
    if (mark != nullptr) {
        inputRow->push(mark);
    } else {
        // Reserve a fixed-width slot identical to the mark widget so a markless
        // input keeps the exact same width as a marked sibling input.
        auto *reserve = new QWidget;
        reserve->setFixedWidth(metric::VALIDATION_MARK_WIDTH);
        inputRow->push(reserve);
    }

    // Keep the label tight against its input.
    (new qontrol::Column)
        ->spacing(resolve(Spacing::XXS))
        ->push(labelWidget)
        ->push(inputRow)
        ->into(this);
}

auto LabelledInput::qss(const Palette &p) -> QString {
    // More specific than the base caption color rule, so secondary wins.
    return QString("QLabel[class=\"label\"][role=\"caption\"][labelKind=\"labelled\"]"
                   " { color: %1; }")
        .arg(p.textSecondary.name());
}

} // namespace catalog
