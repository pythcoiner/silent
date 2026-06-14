#include "catalog/form/ValidationMark.h"

#include "theme/Theme.h"

namespace catalog {

ValidationMark::ValidationMark(QWidget *parent) : QLabel(parent) {
    setProperty("class", "validation-mark");
    setFixedWidth(metric::VALIDATION_MARK_WIDTH);
    setAlignment(Qt::AlignCenter);
    applyState();
}

void ValidationMark::setState(State state) {
    if (m_state == state) {
        return;
    }
    m_state = state;
    applyState();
}

auto ValidationMark::state() const -> State {
    return m_state;
}

void ValidationMark::applyState() {
    switch (m_state) {
    case State::None:
        setProperty("state", "none");
        setText("");
        break;
    case State::Valid:
        setProperty("state", "valid");
        setText(QString::fromUtf8("✓"));
        break;
    case State::Invalid:
        setProperty("state", "invalid");
        setText(QString::fromUtf8("✗"));
        break;
    }

    const auto &font = Theme::get()->fontPalette().body;
    auto markFont = this->font();
    markFont.setFamily(font.family);
    markFont.setPointSize(font.size);
    markFont.setWeight(QFont::Bold);
    setFont(markFont);
}

auto ValidationMark::qss(const Palette &p) -> QString {
    return QString("QLabel[class=\"validation-mark\"][state=\"valid\"] { color: %1; }"
                   "QLabel[class=\"validation-mark\"][state=\"invalid\"] { color: %2; }")
        .arg(p.success.name())
        .arg(p.error.name());
}

} // namespace catalog
