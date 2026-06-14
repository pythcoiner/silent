#pragma once

#include "theme/Palette.h"
#include <QLabel>

namespace catalog {

class Tooltip : public QLabel {
    Q_OBJECT

public:
    explicit Tooltip(const QString &text, QWidget *parent = nullptr);
    void attachTo(QWidget *widget);
    static auto qss(const Palette &p) -> QString;

protected:
    [[nodiscard]] auto eventFilter(QObject *watched, QEvent *event) -> bool override;
};

} // namespace catalog
