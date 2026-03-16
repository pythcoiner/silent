#include "Theme.h"
#include "Button.h"
#include "Checkbox.h"
#include "ComboBox.h"
#include "Display.h"
#include "Icon.h"
#include "Input.h"
#include "Label.h"
#include "Tab.h"
#include "TextEdit.h"
#include "Toggle.h"
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QStyleOption>

namespace {

class AppStyle : public QProxyStyle {
public:
    explicit AppStyle(QStyle *base) : QProxyStyle(base) {
    }

    auto standardIcon(StandardPixmap sp, const QStyleOption *opt = nullptr,
                      const QWidget *widget = nullptr) const -> QIcon override {
        if (sp == SP_TabCloseButton) {
            return icon::close();
        }
        return QProxyStyle::standardIcon(sp, opt, widget);
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *painter,
                       const QWidget *widget = nullptr) const override {
        if (pe == PE_IndicatorTabClose) {
            constexpr int iconSize = 10;
            auto ic = standardIcon(SP_TabCloseButton, opt, widget);
            auto pix = ic.pixmap(iconSize, iconSize);
            int x = opt->rect.x() + (opt->rect.width() - iconSize) / 2;
            int y = opt->rect.y() + (opt->rect.height() - iconSize) / 2;
            painter->drawPixmap(x, y, pix);
            return;
        }
        QProxyStyle::drawPrimitive(pe, opt, painter, widget);
    }
};

} // namespace

Theme *Theme::s_instance = nullptr;

Theme::Theme(QObject *parent) : QObject(parent) {
}

void Theme::init() {
    if (s_instance != nullptr) {
        return;
    }
    s_instance = new Theme(qApp);
    qApp->setStyle(new AppStyle(QStyleFactory::create("Fusion")));
}

auto Theme::get() -> Theme * {
    return s_instance;
}

void Theme::setMode(ThemeMode mode) {
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    m_palette = (mode == ThemeMode::Dark) ? Palette::dark() : Palette::light();
    m_font_palette = (mode == ThemeMode::Dark) ? FontPalette::dark() : FontPalette::light();
    m_button_palette = (mode == ThemeMode::Dark) ? ButtonPalette::dark() : ButtonPalette::light();
    m_icon_palette = (mode == ThemeMode::Dark) ? IconPalette::dark() : IconPalette::light();
    m_input_palette = (mode == ThemeMode::Dark) ? InputPalette::dark() : InputPalette::light();
    apply();
    emit themeChanged(m_mode);
}

auto Theme::mode() const -> ThemeMode {
    return m_mode;
}

auto Theme::palette() const -> const Palette & {
    return m_palette;
}

auto Theme::fontPalette() const -> const FontPalette & {
    return m_font_palette;
}

auto Theme::buttonPalette() const -> const ButtonPalette & {
    return m_button_palette;
}

auto Theme::iconPalette() const -> const IconPalette & {
    return m_icon_palette;
}

auto Theme::inputPalette() const -> const InputPalette & {
    return m_input_palette;
}

void Theme::apply() {
    const auto &p = m_palette;

    // Build QPalette — Fusion style respects all these roles
    QPalette pal;

    pal.setColor(QPalette::Window, p.bg);
    pal.setColor(QPalette::WindowText, p.text);
    pal.setColor(QPalette::Base, p.inputBg);
    pal.setColor(QPalette::AlternateBase, p.surface);
    pal.setColor(QPalette::ToolTipBase, p.surface);
    pal.setColor(QPalette::ToolTipText, p.text);
    pal.setColor(QPalette::Text, p.text);
    pal.setColor(QPalette::Button, p.bgSecondary);
    pal.setColor(QPalette::ButtonText, p.text);
    pal.setColor(QPalette::BrightText, p.error);
    pal.setColor(QPalette::Link, p.accent);
    pal.setColor(QPalette::Highlight, p.accent);
    pal.setColor(QPalette::HighlightedText, color::WHITE);
    pal.setColor(QPalette::PlaceholderText, p.textMuted);
    pal.setColor(QPalette::Light, p.surfaceHover);
    pal.setColor(QPalette::Midlight, p.borderLight);
    pal.setColor(QPalette::Mid, p.border);
    pal.setColor(QPalette::Dark, p.bgSecondary);
    pal.setColor(QPalette::Shadow, p.bg);

    // Disabled state
    pal.setColor(QPalette::Disabled, QPalette::WindowText, p.textMuted);
    pal.setColor(QPalette::Disabled, QPalette::Text, p.textMuted);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, p.textMuted);

    qApp->setPalette(pal);

    // Supplementary QSS for elements QPalette doesn't fully cover
    auto qss = QString(
                   // --- Table ---
                   "QHeaderView::section { background: %1; color: %2; padding: 4px 8px; "
                   "  border: 1px solid %3; }"
                   "QTableWidget { gridline-color: %4; }"
                   // --- ScrollArea ---
                   "QScrollArea { border: none; }"
                   // --- Tooltip ---
                   "QToolTip { background: %5; color: %6; border: 1px solid %3; padding: 4px; }")
                   .arg(p.bgSecondary.name())   // %1
                   .arg(p.textSecondary.name()) // %2
                   .arg(p.border.name())        // %3
                   .arg(p.borderLight.name())   // %4
                   .arg(p.surface.name())       // %5
                   .arg(p.text.name());         // %6

    qss += theme::Tab::qss(p);
    qss += theme::Button::qss(p);
    qss += theme::Label::qss(p);
    qss += theme::Input::qss(p);
    qss += theme::Display::qss(p);
    qss += theme::Checkbox::qss(p);
    qss += theme::ComboBox::qss(p);
    qss += theme::TextEdit::qss(p);

    qApp->setStyleSheet(qss);
}
