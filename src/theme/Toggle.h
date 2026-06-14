#pragma once

#include <QColor>
#include <Qontrol>

struct Palette;

namespace theme {

enum class ToggleRole { Default, Status };

class Toggle : public qontrol::widgets::ToggleSwitch {
    Q_OBJECT

public:
    explicit Toggle(ToggleRole role = ToggleRole::Default, QWidget *parent = nullptr);
    void setRole(ToggleRole role);
    [[nodiscard]] auto role() const -> ToggleRole;
    static auto qss(const Palette &p) -> QString;

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    // The whole switch body is clickable. QCheckBox's default hit region is the
    // style's indicator rect, which for this text-less custom switch is a tiny
    // left strip, so clicks on the visible body never register as a press.
    [[nodiscard]] auto hitButton(const QPoint &pos) const -> bool override;

private:
    ToggleRole m_role = ToggleRole::Default;
    QColor m_color_on;
    QColor m_color_off;
    QColor m_handle;
    void applyRole();
};

} // namespace theme
