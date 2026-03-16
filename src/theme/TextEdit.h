#pragma once

#include "Palette.h"
#include <QTextEdit>
#include <QWidget>

namespace theme {

class Label;

enum class TextEditRole { Default, Mono };

class TextEdit : public QWidget {
    Q_OBJECT

public:
    explicit TextEdit(const QString &label, TextEditRole role = TextEditRole::Default,
                      QWidget *parent = nullptr);
    explicit TextEdit(TextEditRole role = TextEditRole::Default, QWidget *parent = nullptr);

    void setRole(TextEditRole role);
    [[nodiscard]] auto role() const -> TextEditRole;

    auto textEdit() -> QTextEdit *;
    auto label() -> Label *;

    void setText(const QString &text);
    [[nodiscard]] auto toPlainText() const -> QString;
    void setPlaceholderText(const QString &text);
    void setMaximumHeight(int h);
    void setWidth(Size s);
    void setMinimumWidth(int w);

    static auto qss(const Palette &p) -> QString;

signals:
    void textChanged();

protected:
    void applyRole();

private:
    TextEditRole m_role = TextEditRole::Default;
    QTextEdit *m_text_edit = nullptr;
    Label *m_label = nullptr;
};

} // namespace theme
