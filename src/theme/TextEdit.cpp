#include "TextEdit.h"
#include "Label.h"
#include "Palette.h"
#include "Theme.h"
#include <QHBoxLayout>

namespace theme {

TextEdit::TextEdit(const QString &label, TextEditRole role, QWidget *parent)
    : QWidget(parent),
      m_role(role) {
    m_label = new Label(label, LabelRole::InputLabel, this);

    m_text_edit = new QTextEdit(this);
    m_text_edit->setProperty("class", "textedit");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_label);
    layout->addWidget(m_text_edit);

    connect(m_text_edit, &QTextEdit::textChanged, this, &TextEdit::textChanged);
    applyRole();
}

TextEdit::TextEdit(TextEditRole role, QWidget *parent) : QWidget(parent), m_role(role) {
    m_text_edit = new QTextEdit(this);
    m_text_edit->setProperty("class", "textedit");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_text_edit);

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
                   "  border-radius: 6px;"
                   "  padding: 6px 10px;"
                   "  color: %3;"
                   "}"
                   "QTextEdit[class=\"textedit\"]:focus {"
                   "  border: 1px solid %4;"
                   "}")
        .arg(p.inputBorder.name()) // %1
        .arg(p.inputBg.name())     // %2
        .arg(p.text.name())        // %3
        .arg(p.inputFocus.name()); // %4
}

} // namespace theme
