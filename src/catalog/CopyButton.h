#pragma once

#include "catalog/Button.h"
#include <QString>

namespace catalog {

// An icon button that copies a string to the clipboard and flashes a check mark
// before reverting to the copy icon. Comes in two sizes (Small -> inline icon,
// Large -> icon). Reuse this everywhere a copy button is needed.
class CopyButton : public Button {
    Q_OBJECT

public:
    enum class Size { Small, Large };

    explicit CopyButton(Size size = Size::Large, QWidget *parent = nullptr);
    // The text placed on the clipboard when the button is clicked.
    void setTextToCopy(const QString &text);

signals:
    void copied(const QString &text);

private slots:
    void onCopyClicked();

private:
    QString m_text;
};

} // namespace catalog
