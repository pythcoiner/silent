#pragma once

#include <QColor>
#include <QIcon>

auto renderIcon(const char *svg_data, int size, const QColor &color, int stroke_width = 2) -> QIcon;

namespace icon {

auto trash() -> QIcon;
auto close() -> QIcon;
auto coins() -> QIcon;
auto send() -> QIcon;
auto receive() -> QIcon;
auto settings() -> QIcon;
auto pencil() -> QIcon;

} // namespace icon
