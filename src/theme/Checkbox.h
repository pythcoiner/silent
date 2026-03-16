#pragma once

#include <QCheckBox>
#include <QString>

struct Palette;

namespace theme {

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

private:
    CheckboxRole m_role = CheckboxRole::Default;
    void applyRole();
};

} // namespace theme
