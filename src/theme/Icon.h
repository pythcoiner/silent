#pragma once

#include <QIcon>

// Builds a QIcon that re-renders the SVG with the live theme foreground color on
// every paint, so icons follow theme (light/dark) changes without recreation.
auto renderIcon(const char *svg_data, int stroke_width = 2) -> QIcon;

namespace icon {

auto trash() -> QIcon;
auto close() -> QIcon;
auto coins() -> QIcon;
auto send() -> QIcon;
auto receive() -> QIcon;
auto settings() -> QIcon;
auto pencil() -> QIcon;
auto history() -> QIcon;

} // namespace icon
