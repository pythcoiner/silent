#include "catalog/CopyButton.h"

#include "theme/Icon.h"

#include <QApplication>
#include <QClipboard>

namespace catalog {

CopyButton::CopyButton(Size size, QWidget *parent)
    : Button(size == Size::Large ? ButtonRole::Icon : ButtonRole::InlineIcon, parent) {
    setIcon(icon::copy());
    setFeedbackIcon(icon::check()); // flash a check on copy, then revert
    connect(this, &QPushButton::clicked, this, &CopyButton::onCopyClicked);
}

void CopyButton::setTextToCopy(const QString &text) {
    m_text = text;
}

void CopyButton::onCopyClicked() {
    QApplication::clipboard()->setText(m_text);
    emit copied(m_text);
}

} // namespace catalog
