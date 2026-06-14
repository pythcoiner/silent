#include "Tab.h"
#include "Palette.h"

namespace theme {

auto Tab::qss(const Palette &p) -> QString {
    return QString("QTabWidget::pane { border: 1px solid %1; }"
                   // Pull the top-right corner widget flush to the edge; the style otherwise
                   // insets it by a fixed 24px.
                   "QTabWidget::right-corner { right: -24px; }"
                   "QTabBar::tab { background: %2; color: %3; padding: 6px 16px; "
                   "  border: 1px solid %1; border-bottom: none; border-top-left-radius: 4px; "
                   "  border-top-right-radius: 4px; margin-right: 2px; }"
                   "QTabBar::tab:selected { background: %4; color: %5; }"
                   "QTabBar::tab:hover:!selected { background: %6; }")
        .arg(p.border.name())        // %1
        .arg(p.bgSecondary.name())   // %2
        .arg(p.textSecondary.name()) // %3
        .arg(p.bg.name())            // %4
        .arg(p.text.name())          // %5
        .arg(p.surfaceHover.name()); // %6
}

} // namespace theme
