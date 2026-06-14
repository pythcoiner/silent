#include "catalog/panels/receive/IndexPill.h"

#include "catalog/display/Label.h"
#include "theme/Icon.h"
#include "theme/Palette.h"

#include <QMouseEvent>
#include <QStyle>
#include <Qontrol>
#include <common.h>

namespace catalog {

namespace {
constexpr int PILL_WIDTH = 60;
constexpr int PILL_HEIGHT = 30;
constexpr int ICON_SIZE = 14;
} // namespace

IndexPill::IndexPill(QWidget *parent) : QWidget(parent) {
    setProperty("class", "index-pill");
    setProperty("active", false);
    setProperty("pressed", false);
    // A plain QWidget only paints its stylesheet background/border with this flag,
    // and only reacts to :hover with WA_Hover.
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(PILL_WIDTH, PILL_HEIGHT);

    auto *iconLabel = new QLabel(this);
    iconLabel->setPixmap(icon::qr().pixmap(ICON_SIZE, ICON_SIZE));

    m_index = new Label(LabelRole::Mono, this);

    (new qontrol::Row)
        ->margins(resolve(Spacing::S), 0, resolve(Spacing::S), 0)
        ->spacing(resolve(Spacing::XS))
        ->push(iconLabel)
        ->push(m_index)
        ->into(this);
}

void IndexPill::setIndex(int index) {
    m_index->setText(QString::number(index));
}

void IndexPill::setActive(bool active) {
    setStateProperty("active", active);
}

void IndexPill::setStateProperty(const char *name, bool value) {
    setProperty(name, value);
    style()->unpolish(this);
    style()->polish(this);
}

void IndexPill::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        setStateProperty("pressed", true);
        return;
    }
    QWidget::mousePressEvent(event);
}

void IndexPill::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        const bool inside = rect().contains(event->pos());
        setStateProperty("pressed", false);
        if (inside) {
            emit clicked();
        }
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

auto IndexPill::qss(const Palette &p) -> QString {
    return QString(
               "QWidget[class=\"index-pill\"] { background: %1; border: 1px solid %2; "
               "border-radius: %3px; }"
               "QWidget[class=\"index-pill\"]:hover { background: %4; }"
               "QWidget[class=\"index-pill\"][pressed=\"true\"] { background: %5; }"
               "QWidget[class=\"index-pill\"][active=\"true\"] { background: %5; }")
        .arg(p.bgSecondary.name()) // %1 default background
        .arg(p.border.name())      // %2 border
        .arg(radius::BUTTON)       // %3
        .arg(p.surfaceHover.name()) // %4 hover background
        .arg(p.bg.name());          // %5 pressed / active background
}

} // namespace catalog
