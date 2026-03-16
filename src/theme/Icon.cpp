#include "Icon.h"
#include "Theme.h"
#include "resources/icon/coins.h"
#include "resources/icon/download.h"
#include "resources/icon/settings.h"
#include "resources/icon/trash_2.h"
#include "resources/icon/upload.h"
#include "resources/icon/pencil.h"
#include "resources/icon/x.h"
#include <QByteArray>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QSvgRenderer>

auto renderIcon(const char *svg_data, int size, const QColor &color, int stroke_width) -> QIcon {
    auto svg = QString(svg_data);
    svg.replace("stroke=\"currentColor\"", QString("stroke=\"%1\"").arg(color.name()));
    svg.replace("stroke-width=\"2\"", QString("stroke-width=\"%1\"").arg(stroke_width));

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return QIcon(pixmap);
}

namespace icon {

auto trash() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::TRASH_2, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

auto close() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::X, ip.defaultIcon.size, p.text, ip.defaultIcon.strokeWidth);
}

auto coins() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::COINS, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

auto send() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::UPLOAD, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

auto receive() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::DOWNLOAD, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

auto settings() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::SETTINGS, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

auto pencil() -> QIcon {
    auto *theme = Theme::get();
    const auto &p = theme->palette();
    const auto &ip = theme->iconPalette();
    return renderIcon(embedded_icon::PENCIL, ip.defaultIcon.size, p.text,
                      ip.defaultIcon.strokeWidth);
}

} // namespace icon
