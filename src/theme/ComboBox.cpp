#include "ComboBox.h"
#include "Palette.h"
#include "Theme.h"
#include "resources/icon/chevron_down.h"
#include <QAbstractItemView>
#include <QDir>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStandardPaths>
#include <QStyle>
#include <QSvgRenderer>

namespace theme {

ComboBox::ComboBox(ComboBoxRole role, QWidget *parent) : QComboBox(parent), m_role(role) {
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

    constexpr int radius = 8;
    QPainterPath path;
    path.addRoundedRect(container->rect(), radius, radius);
    container->setMask(path.toFillPolygon().toPolygon());
}

auto ComboBox::renderChevron(const QColor &color) -> QString {
    static QString s_cached_path;
    static QColor s_cached_color;

    if (!s_cached_path.isEmpty() && s_cached_color == color) {
        return s_cached_path;
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
    s_cached_path = dir + "/silent_chevron_down.png";
    pixmap.save(s_cached_path);
    s_cached_color = color;
    return s_cached_path;
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
