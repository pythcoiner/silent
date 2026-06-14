#pragma once

#include <QCheckBox>
#include <QString>

struct Palette;

namespace catalog {

enum class CheckboxRole { Default };

class Checkbox : public QCheckBox {
    Q_OBJECT

public:
    explicit Checkbox(const QString &text, CheckboxRole role = CheckboxRole::Default,
                      QWidget *parent = nullptr);
    explicit Checkbox(CheckboxRole role = CheckboxRole::Default, QWidget *parent = nullptr);
    void setRole(CheckboxRole role);
    [[nodiscard]] auto role() const -> CheckboxRole;
    static auto qss(const Palette &p) -> QString;

protected:
    void paintEvent(QPaintEvent *e) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;
    // Make the whole widget clickable; QCheckBox's default hit rect is the style's
    // small indicator area, which excludes the painted box's edges.
    [[nodiscard]] auto hitButton(const QPoint &pos) const -> bool override;

private:
    CheckboxRole m_role = CheckboxRole::Default;
    void applyRole();
};

} // namespace catalog
