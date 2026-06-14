#include "catalog/feedback/Tooltip.h"

#include "theme/Palette.h"
#include "theme/Theme.h"

#include <QEvent>
#include <QWidget>

namespace catalog {

Tooltip::Tooltip(const QString &text, QWidget *parent) : QLabel(text, parent) {
    setProperty("class", "tooltip");
    setWindowFlags(Qt::ToolTip);
    setContentsMargins(resolve(Padding::S), resolve(Spacing::XS), resolve(Padding::S),
                       resolve(Spacing::XS));

    const auto &fp = Theme::get()->fontPalette().caption;
    auto f = font();
    f.setFamily(fp.family);
    f.setPointSize(fp.size);
    f.setWeight(QFont::Medium);
    setFont(f);

    hide();
}

void Tooltip::attachTo(QWidget *widget) {
    widget->installEventFilter(this);
}

auto Tooltip::eventFilter(QObject *watched, QEvent *event) -> bool {
    auto *widget = qobject_cast<QWidget *>(watched);
    if (widget == nullptr) {
        return QLabel::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Enter) {
        move(widget->mapToGlobal(QPoint(0, widget->height() + resolve(Spacing::XS))));
        show();
    }
    if (event->type() == QEvent::Leave) {
        hide();
    }

    return QLabel::eventFilter(watched, event);
}

auto Tooltip::qss(const Palette &p) -> QString {
    return QString("QLabel[class=\"tooltip\"] { background: %1; color: %2; border-radius: %3px; }")
        .arg(p.text.name())
        .arg(p.bg.name())
        .arg(radius::ICON);
}

} // namespace catalog
