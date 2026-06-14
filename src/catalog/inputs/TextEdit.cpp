#include "catalog/inputs/TextEdit.h"
#include "catalog/display/Label.h"
#include "theme/Palette.h"
#include "theme/Theme.h"
#include <Qontrol>

namespace catalog {

TextEdit::TextEdit(const QString &label, TextEditRole role, QWidget *parent)
    : QWidget(parent),
      m_role(role) {
    m_label = new Label(label, LabelRole::InputLabel, this);

    m_text_edit = new QTextEdit(this);
    m_text_edit->setProperty("class", "textedit");

    (new qontrol::Row)->push(m_label)->push(m_text_edit)->into(this);

    connect(m_text_edit, &QTextEdit::textChanged, this, &TextEdit::textChanged);
    applyRole();
}

TextEdit::TextEdit(TextEditRole role, QWidget *parent) : QWidget(parent), m_role(role) {
    m_text_edit = new QTextEdit(this);
    m_text_edit->setProperty("class", "textedit");

    (new qontrol::Row)->push(m_text_edit)->into(this);

    connect(m_text_edit, &QTextEdit::textChanged, this, &TextEdit::textChanged);
    applyRole();
}

void TextEdit::setRole(TextEditRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
}

auto TextEdit::role() const -> TextEditRole {
    return m_role;
}

auto TextEdit::textEdit() -> QTextEdit * {
    return m_text_edit;
}

auto TextEdit::label() -> Label * {
    return m_label;
}

void TextEdit::setText(const QString &text) {
    m_text_edit->setPlainText(text);
}

auto TextEdit::toPlainText() const -> QString {
    return m_text_edit->toPlainText();
}

void TextEdit::setPlaceholderText(const QString &text) {
    m_text_edit->setPlaceholderText(text);
}

void TextEdit::setMaximumHeight(int h) {
    m_text_edit->setMaximumHeight(h);
}

void TextEdit::setWidth(Size s) {
    m_text_edit->setMinimumWidth(resolve(s));
}

void TextEdit::setMinimumWidth(int w) {
    m_text_edit->setMinimumWidth(w);
}

void TextEdit::applyRole() {
    const auto &ip = Theme::get()->inputPalette();

    const auto *style = &ip.defaultInput;
    switch (m_role) {
    case TextEditRole::Default:
        style = &ip.defaultInput;
        m_text_edit->setProperty("role", "default");
        break;
    case TextEditRole::Mono:
        style = &ip.mono;
        m_text_edit->setProperty("role", "mono");
        break;
    }

    auto f = m_text_edit->font();
    f.setPointSize(style->font.size);
    f.setFamily(style->font.family);
    f.setWeight(style->font.weight);
    m_text_edit->setFont(f);
}

auto TextEdit::qss(const Palette &p) -> QString {
    return QString("QTextEdit[class=\"textedit\"] {"
                   "  border: 1px solid %1;"
                   "  background: %2;"
                   "  border-radius: %3px;"
                   "  padding: %4px %5px;"
                   "  color: %6;"
                   "}"
                   "QTextEdit[class=\"textedit\"]:focus {"
                   "  border: 1px solid %7;"
                   "}"
                   "QTextEdit[class=\"textedit\"]:disabled {"
                   "  background: %8;"
                   "  color: %9;"
                   "  border-color: %10;"
                   "}")
        .arg(p.inputBorder.name())   // %1
        .arg(p.inputBg.name())       // %2
        .arg(radius::INPUT)          // %3
        .arg(resolve(Padding::XS))   // %4
        .arg(resolve(Padding::S))    // %5
        .arg(p.text.name())          // %6
        .arg(p.inputFocus.name())    // %7
        .arg(p.bgSecondary.name())   // %8
        .arg(p.textSecondary.name()) // %9
        .arg(p.borderLight.name());  // %10
}

} // namespace catalog
