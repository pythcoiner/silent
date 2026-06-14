#include "catalog/containers/Tab.h"
#include "theme/Palette.h"

namespace catalog {

auto Tab::qss(const Palette &p) -> QString {
    return QString("QTabWidget::pane { border: 1px solid %1; }"
                   // Pull the top-right corner widget flush to the edge; the style otherwise
                   // insets it by a fixed 24px.
                   "QTabWidget::right-corner { right: -%7px; }"
                   "QTabBar::tab { background: %2; color: %3; padding: %8px %9px; "
                   "  font-size: %12pt; "
                   "  border: 1px solid %1; border-bottom: none; border-top-left-radius: %10px; "
                   "  border-top-right-radius: %10px; margin-right: %11px; }"
                   "QTabBar::tab:selected { background: %4; color: %5; }"
                   "QTabBar::tab:hover:!selected { background: %6; }")
        .arg(p.border.name())        // %1
        .arg(p.bgSecondary.name())   // %2
        .arg(p.textSecondary.name()) // %3
        .arg(p.bg.name())            // %4
        .arg(p.text.name())          // %5
        .arg(p.surfaceHover.name())  // %6
        .arg(metric::ICON_SIZE)      // %7
        .arg(resolve(Padding::XS))   // %8
        .arg(resolve(Padding::M))    // %9
        .arg(radius::TAB)            // %10
        .arg(resolve(Spacing::XXS))  // %11
        .arg(size::BODY);            // %12
}

} // namespace catalog
