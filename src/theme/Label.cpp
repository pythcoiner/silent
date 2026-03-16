#include "Label.h"
#include "Palette.h"
#include "Theme.h"
#include <QFont>
#include <QStyle>

namespace theme {

Label::Label(const QString &text, LabelRole role, QWidget *parent)
    : QLabel(text, parent),
      m_role(role) {
    applyRole();
}

Label::Label(LabelRole role, QWidget *parent) : QLabel(parent), m_role(role) {
    applyRole();
}

void Label::setRole(LabelRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto Label::role() const -> LabelRole {
    return m_role;
}

void Label::setScale(qreal scale) {
    if (qFuzzyCompare(m_scale, scale)) {
        return;
    }
    m_scale = scale;
    applyRole();
}

auto Label::scale() const -> qreal {
    return m_scale;
}

static auto fontForRole(const FontPalette &fp, LabelRole role) -> const Font & {
    switch (role) {
    case LabelRole::Title:
        return fp.title;
    case LabelRole::Heading:
        return fp.heading;
    case LabelRole::Subheading:
        return fp.subheading;
    case LabelRole::Section:
        return fp.section;
    case LabelRole::InputLabel:
        return fp.inputLabel;
    case LabelRole::CheckboxLabel:
        return fp.checkboxLabel;
    case LabelRole::InfoLabel:
        return fp.infoLabel;
    case LabelRole::Info:
        return fp.info;
    case LabelRole::Body:
        return fp.body;
    case LabelRole::Caption:
        return fp.caption;
    case LabelRole::Mono:
        return fp.mono;
    case LabelRole::Status:
        return fp.status;
    }
    return fp.body;
}

void Label::applyRole() {
    setProperty("class", "label");

    switch (m_role) {
    case LabelRole::Body:
        setProperty("role", "body");
        break;
    case LabelRole::Title:
        setProperty("role", "title");
        break;
    case LabelRole::Heading:
        setProperty("role", "heading");
        break;
    case LabelRole::Subheading:
        setProperty("role", "subheading");
        break;
    case LabelRole::Section:
        setProperty("role", "section");
        break;
    case LabelRole::InputLabel:
        setProperty("role", "input-label");
        break;
    case LabelRole::CheckboxLabel:
        setProperty("role", "checkbox-label");
        break;
    case LabelRole::InfoLabel:
        setProperty("role", "info-label");
        break;
    case LabelRole::Info:
        setProperty("role", "info");
        break;
    case LabelRole::Caption:
        setProperty("role", "caption");
        break;
    case LabelRole::Mono:
        setProperty("role", "mono");
        break;
    case LabelRole::Status:
        setProperty("role", "status");
        break;
    }

    const auto &fp = Theme::get()->fontPalette();
    const auto &roleFont = fontForRole(fp, m_role);

    auto f = font();
    int size = static_cast<int>(roleFont.size * m_scale);
    f.setPointSize(size);
    f.setFamily(roleFont.family);
    f.setWeight(roleFont.weight);

    setFont(f);

    if (roleFont.width != width::NONE) {
        setFixedWidth(roleFont.width);
    } else {
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

auto Label::qss(const Palette &p) -> QString {
    return QString("QLabel[class=\"label\"][role=\"caption\"] { color: %1; }"
                   "QLabel[class=\"label\"][role=\"status\"] { color: %1; }")
        .arg(p.textMuted.name()); // %1
}

} // namespace theme
