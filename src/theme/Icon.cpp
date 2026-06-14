#include "Icon.h"
#include "Theme.h"
#include "resources/icon/coins.h"
#include "resources/icon/download.h"
#include "resources/icon/settings.h"
#include "resources/icon/trash_2.h"
#include "resources/icon/upload.h"
#include "resources/icon/history.h"
#include "resources/icon/pencil.h"
#include "resources/icon/x.h"
#include <QIconEngine>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QString>
#include <QSvgRenderer>

namespace {
// Renders an embedded SVG with the current theme foreground color at paint time.
// Used through a QIcon so widgets follow theme changes without re-creating icons.
class SvgIconEngine final : public QIconEngine {
public:
    SvgIconEngine(const char *svg, int stroke_width) : m_svg(svg), m_stroke_width(stroke_width) {}

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        auto *theme = Theme::get();
        QColor color = theme != nullptr ? theme->palette().text : QColor(Qt::black);
        QString svg = QString::fromUtf8(m_svg);
        svg.replace(QStringLiteral("stroke=\"currentColor\""),
                    QStringLiteral("stroke=\"%1\"").arg(color.name()));
        svg.replace(QStringLiteral("stroke-width=\"2\""),
                    QStringLiteral("stroke-width=\"%1\"").arg(m_stroke_width));
        QSvgRenderer renderer(svg.toUtf8());
        renderer.render(painter, QRectF(rect));
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        paint(&painter, QRect(QPoint(0, 0), size), mode, state);
        return pixmap;
    }

    [[nodiscard]] QIconEngine *clone() const override {
        return new SvgIconEngine(m_svg, m_stroke_width);
    }

private:
    const char *m_svg;
    int m_stroke_width;
};
} // namespace

auto renderIcon(const char *svg_data, int stroke_width) -> QIcon {
    return QIcon(new SvgIconEngine(svg_data, stroke_width));
}

namespace icon {

auto trash() -> QIcon {
    return renderIcon(embedded_icon::TRASH_2, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto close() -> QIcon {
    return renderIcon(embedded_icon::X, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto coins() -> QIcon {
    return renderIcon(embedded_icon::COINS, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto send() -> QIcon {
    return renderIcon(embedded_icon::UPLOAD, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto receive() -> QIcon {
    return renderIcon(embedded_icon::DOWNLOAD, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto settings() -> QIcon {
    return renderIcon(embedded_icon::SETTINGS, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto pencil() -> QIcon {
    return renderIcon(embedded_icon::PENCIL, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

auto history() -> QIcon {
    return renderIcon(embedded_icon::HISTORY, Theme::get()->iconPalette().defaultIcon.strokeWidth);
}

} // namespace icon
