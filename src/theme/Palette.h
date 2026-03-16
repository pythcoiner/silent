#pragma once

#include <QColor>
#include <QFont>
#include <QString>

// ===== Color Constants =====

namespace color {

// Neutrals — dark
const QColor GRAY_95 = QColor(30, 30, 30);
const QColor GRAY_90 = QColor(40, 40, 40);
const QColor GRAY_85 = QColor(50, 50, 50);
const QColor GRAY_80 = QColor(60, 60, 60);
const QColor GRAY_70 = QColor(80, 80, 80);
const QColor GRAY_55 = QColor(110, 110, 110);
const QColor GRAY_40 = QColor(150, 150, 150);
const QColor GRAY_35 = QColor(170, 170, 170);
const QColor GRAY_20 = QColor(190, 190, 190);
const QColor GRAY_15 = QColor(210, 210, 210);
const QColor GRAY_10 = QColor(230, 230, 230);
const QColor GRAY_8 = QColor(235, 235, 235);
const QColor GRAY_5 = QColor(240, 240, 240);
const QColor GRAY_4 = QColor(245, 245, 245);

// Pure
const QColor WHITE = QColor(255, 255, 255);
const QColor BLACK = QColor(30, 30, 30);

// Neutrals — light
const QColor LIGHT_GRAY_65 = QColor(90, 90, 90);

// Blue
const QColor BLUE = QColor(80, 140, 220);
const QColor BLUE_LIGHT = QColor(100, 160, 240);
const QColor BLUE_DARK = QColor(50, 120, 200);
const QColor BLUE_DARK_LIGHT = QColor(70, 140, 220);

// Green
const QColor GREEN = QColor(80, 190, 80);
const QColor GREEN_DARK = QColor(40, 160, 40);

// Red
const QColor RED = QColor(220, 70, 70);
const QColor RED_DARK = QColor(200, 50, 50);

// Yellow
const QColor YELLOW = QColor(220, 180, 50);
const QColor YELLOW_DARK = QColor(200, 160, 30);

} // namespace color

// ===== Size Constants =====

namespace size {

const int H1 = 22;
const int H2 = 16;
const int H3 = 14;
const int H4 = 12;
const int SECTION = 11;
const int BODY = 10;
const int CAPTION = 8;

} // namespace size

// ===== Font Constants =====

namespace font {

const QString DEFAULT = "Noto Sans";
const QString MONO = "monospace";

} // namespace font

// ===== Size (widget widths) =====

enum class Size { XXS, XS, S, M, L, XL, XXL, XXXL };

constexpr auto resolve(Size s) -> int {
    switch (s) {
    case Size::XXS:
        return 50;
    case Size::XS:
        return 100;
    case Size::S:
        return 150;
    case Size::M:
        return 200;
    case Size::L:
        return 300;
    case Size::XL:
        return 400;
    case Size::XXL:
        return 600;
    case Size::XXXL:
        return 1000;
    }
    return 200;
}

// ===== Spacing =====

enum class Spacing { XXS, XS, S, M, L, XL, XXL };

constexpr auto resolve(Spacing s) -> int {
    switch (s) {
    case Spacing::XXS:
        return 2;
    case Spacing::XS:
        return 5;
    case Spacing::S:
        return 10;
    case Spacing::M:
        return 20;
    case Spacing::L:
        return 30;
    case Spacing::XL:
        return 60;
    case Spacing::XXL:
        return 120;
    }
    return 20;
}

// ===== Padding =====

enum class Padding { XXS, XS, S, M, L, XL, XXL };

constexpr auto resolve(Padding p) -> int {
    switch (p) {
    case Padding::XXS:
        return 2;
    case Padding::XS:
        return 6;
    case Padding::S:
        return 10;
    case Padding::M:
        return 15;
    case Padding::L:
        return 20;
    case Padding::XL:
        return 30;
    case Padding::XXL:
        return 50;
    }
    return 20;
}

// ===== Width sentinel =====

namespace width {
const int NONE = -1;
} // namespace width

// ===== Color Palette =====

struct Palette {
    // Backgrounds
    QColor bg;
    QColor bgSecondary;
    QColor surface;
    QColor surfaceHover;

    // Text
    QColor text;
    QColor textSecondary;
    QColor textMuted;

    // Brand / Accent
    QColor accent;
    QColor accentHover;

    // Semantic
    QColor success;
    QColor error;
    QColor warning;

    // Borders
    QColor border;
    QColor borderLight;

    // Inputs
    QColor inputBg;
    QColor inputBorder;
    QColor inputFocus;

    static auto light() -> Palette;
    static auto dark() -> Palette;
};

// ===== Typography =====

struct Font {
    QString family;
    int size;
    QFont::Weight weight;
    int width = width::NONE;
};

struct ButtonStyle {
    Font font;
    int width = width::NONE;
    int height = width::NONE;
    int paddingH = 16;
    int paddingV = 6;
    int borderRadius = 8;
};

struct ButtonPalette {
    ButtonStyle defaultBtn;
    ButtonStyle primary;
    ButtonStyle destructive;
    ButtonStyle menu;
    ButtonStyle icon;
    ButtonStyle inlineIcon;
    ButtonStyle tabOpen;
    ButtonStyle tabCreate;
    ButtonStyle inlineBtn;

    static auto defaultButtonPalette() -> ButtonPalette;
    static auto dark() -> ButtonPalette;
    static auto light() -> ButtonPalette;
};

struct FontPalette {
    Font title;
    Font heading;
    Font subheading;
    Font section;
    Font inputLabel;
    Font checkboxLabel;
    Font infoLabel;
    Font info;
    Font body;
    Font caption;
    Font mono;
    Font status;

    static auto defaultFontPalette() -> FontPalette;
    static auto dark() -> FontPalette;
    static auto light() -> FontPalette;
};

struct IconStyle {
    int size;
    int strokeWidth;
};

struct InputStyle {
    Font font;
};

struct InputPalette {
    InputStyle defaultInput;
    InputStyle mono;
    InputStyle display;

    static auto defaultInputPalette() -> InputPalette;
    static auto dark() -> InputPalette;
    static auto light() -> InputPalette;
};

struct IconPalette {
    IconStyle defaultIcon;

    static auto defaultIconPalette() -> IconPalette;
    static auto dark() -> IconPalette;
    static auto light() -> IconPalette;
};
