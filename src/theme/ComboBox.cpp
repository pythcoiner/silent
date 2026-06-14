#include "ComboBox.h"
#include "Palette.h"
#include "Theme.h"
#include "resources/icon/chevron_down.h"
#include <QAbstractItemView>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QPainter>
#include <QPixmap>
#include <QProxyStyle>
#include <QStandardPaths>
#include <QStyle>
#include <QSvgRenderer>

namespace theme {

namespace {
// Forces a plain list popup instead of the menu-style one, so Qt never creates the
// top/bottom scroll-indicator widgets that otherwise overlay the dropdown as bands.
class PlainPopupStyle final : public QProxyStyle {
public:
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                  QStyleHintReturn *return_data) const override {
        if (hint == QStyle::SH_ComboBox_Popup) {
            return 0;
        }
        return QProxyStyle::styleHint(hint, option, widget, return_data);
    }
};
} // namespace

ComboBox::ComboBox(ComboBoxRole role, QWidget *parent) : QComboBox(parent), m_role(role) {
    auto *popupStyle = new PlainPopupStyle;
    popupStyle->setParent(this);
    setStyle(popupStyle);
    applyRole();
}

void ComboBox::setRole(ComboBoxRole role) {
    if (m_role == role) {
        return;
    }
    m_role = role;
    applyRole();
    style()->unpolish(this);
    style()->polish(this);
}

auto ComboBox::role() const -> ComboBoxRole {
    return m_role;
}

void ComboBox::applyRole() {
    setProperty("class", "combobox");

    switch (m_role) {
    case ComboBoxRole::Default:
        setProperty("role", "default");
        break;
    }

    const auto &fp = Theme::get()->fontPalette();
    auto f = font();
    f.setPointSize(fp.body.size);
    f.setFamily(fp.body.family);
    f.setWeight(fp.body.weight);
    setFont(f);

}

void ComboBox::setWidth(Size s) {
    setFixedWidth(resolve(s));
}

void ComboBox::showPopup() {
    QComboBox::showPopup();

    auto *popup = view();
    auto *container = popup->parentWidget();
    if (container == nullptr) {
        return;
    }

    container->setWindowFlag(Qt::FramelessWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setContentsMargins(0, 0, 0, 0);
    if (auto *frame = qobject_cast<QFrame *>(container)) {
        frame->setFrameShape(QFrame::NoFrame);
    }
    // No mask: a 1-bit mask aliases the rounded corners. The translucent window lets the
    // view's QSS border-radius round them with anti-aliasing instead.
}

auto ComboBox::renderChevron(const QColor &color) -> QString {
    static QString sCachedPath;
    static QColor sCachedColor;

    if (!sCachedPath.isEmpty() && sCachedColor == color) {
        return sCachedPath;
    }

    auto svg = QString(embedded_icon::CHEVRON_DOWN);
    svg.replace("stroke=\"currentColor\"", QString("stroke=\"%1\"").arg(color.name()));

    constexpr int size = 12;
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);

    auto dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    sCachedPath = dir + "/silent_chevron_down.png";
    pixmap.save(sCachedPath);
    sCachedColor = color;
    return sCachedPath;
}

auto ComboBox::qss(const Palette &p) -> QString {
    auto chevron = renderChevron(p.text);

    return QString("QComboBox[class=\"combobox\"] {"
                   "  border: 1px solid %1;"
                   "  background: %2;"
                   "  border-radius: 8px;"
                   "  padding: 6px 10px;"
                   "  color: %3;"
                   "}"
                   "QComboBox[class=\"combobox\"]:focus {"
                   "  border: 1px solid %4;"
                   "}"
                   "QComboBox[class=\"combobox\"]::drop-down {"
                   "  border: none;"
                   "  border-left: 1px solid %1;"
                   "  background: %5;"
                   "  border-top-right-radius: 8px;"
                   "  border-bottom-right-radius: 8px;"
                   "  width: 30px;"
                   "}"
                   "QComboBox[class=\"combobox\"]::drop-down:hover {"
                   "  background: %6;"
                   "}"
                   "QComboBox[class=\"combobox\"]::down-arrow {"
                   "  image: url(%7);"
                   "  width: 12px;"
                   "  height: 12px;"
                   "}"
                   "QComboBox[class=\"combobox\"] QAbstractItemView {"
                   "  background: %2;"
                   "  color: %3;"
                   "  border: 1px solid %1;"
                   "  border-radius: 8px;"
                   "  selection-background-color: %6;"
                   "}")
        .arg(p.inputBorder.name())   // %1
        .arg(p.inputBg.name())       // %2
        .arg(p.text.name())          // %3
        .arg(p.inputFocus.name())    // %4
        .arg(p.bgSecondary.name())   // %5
        .arg(p.surfaceHover.name())  // %6
        .arg(chevron);               // %7
}

} // namespace theme
