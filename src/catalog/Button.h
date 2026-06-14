#pragma once

#include <QIcon>
#include <QPushButton>
#include <QString>

class QTimer;
struct Palette;

namespace catalog {

enum class ButtonRole { Default, Primary, Destructive, Menu, Icon, InlineIcon, TabOpen, TabCreate, Inline };

class Button : public QPushButton {
    Q_OBJECT

public:
    // Default time the feedback icon is shown after a click before reverting.
    static constexpr int FEEDBACK_MS = 1400;

    explicit Button(const QString &text, ButtonRole role = ButtonRole::Default,
                    QWidget *parent = nullptr);
    explicit Button(ButtonRole role = ButtonRole::Default, QWidget *parent = nullptr);
    void setRole(ButtonRole role);
    [[nodiscard]] auto role() const -> ButtonRole;
    // Show `icon` transiently for `duration_ms` after each click (e.g. a check
    // mark on a copy button), then revert to the button's current icon.
    void setFeedbackIcon(const QIcon &icon, int duration_ms = FEEDBACK_MS);
    static auto qss(const Palette &p) -> QString;

private slots:
    void onFeedbackClicked();
    void onFeedbackRestore();

private:
    ButtonRole m_role = ButtonRole::Default;
    void applyRole();

    QIcon m_base_icon;
    QIcon m_feedback_icon;
    QTimer *m_feedback_timer = nullptr;
};

} // namespace catalog
