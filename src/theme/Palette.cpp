#include "Palette.h"

// ===== Color Palette =====

auto Palette::dark() -> Palette {
    return Palette{
        // Backgrounds
        .bg = color::GRAY_95,
        .bgSecondary = color::GRAY_90,
        .surface = color::GRAY_85,
        .surfaceHover = color::GRAY_80,

        // Text
        .text = color::GRAY_10,
        .textSecondary = color::GRAY_35,
        .textMuted = color::GRAY_55,

        // Brand / Accent
        .accent = color::BLUE,
        .accentHover = color::BLUE_LIGHT,

        // Semantic
        .success = color::GREEN,
        .error = color::RED,
        .warning = color::YELLOW,

        // Borders
        .border = color::GRAY_70,
        .borderLight = color::GRAY_80,

        // Inputs
        .inputBg = color::GRAY_90,
        .inputBorder = color::GRAY_70,
        .inputFocus = color::BLUE,
    };
}

auto Palette::light() -> Palette {
    return Palette{
        // Backgrounds
        .bg = color::GRAY_4,
        .bgSecondary = color::GRAY_8,
        .surface = color::WHITE,
        .surfaceHover = color::GRAY_5,

        // Text
        .text = color::BLACK,
        .textSecondary = color::LIGHT_GRAY_65,
        .textMuted = color::GRAY_40,

        // Brand / Accent
        .accent = color::BLUE_DARK,
        .accentHover = color::BLUE_DARK_LIGHT,

        // Semantic
        .success = color::GREEN_DARK,
        .error = color::RED_DARK,
        .warning = color::YELLOW_DARK,

        // Borders
        .border = color::GRAY_20,
        .borderLight = color::GRAY_15,

        // Inputs
        .inputBg = color::WHITE,
        .inputBorder = color::GRAY_20,
        .inputFocus = color::BLUE_DARK,
    };
}

// ===== Typography =====

auto ButtonPalette::defaultButtonPalette() -> ButtonPalette {
    return ButtonPalette{
        .defaultBtn = {.font = {.family = font::DEFAULT,
                                .size = size::BODY,
                                .weight = QFont::Normal}},
        .primary = {.font = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal}},
        .destructive = {.font = {.family = font::DEFAULT,
                                 .size = size::BODY,
                                 .weight = QFont::Normal}},
        .menu = {.font = {.family = font::DEFAULT, .size = size::H3, .weight = QFont::Normal},
                 .paddingH = 16,
                 .paddingV = 12,
                 .borderRadius = 8},
        .icon = {.font = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal},
                 .width = 50,
                 .height = 50,
                 .paddingH = 0,
                 .paddingV = 0},
        .inlineIcon = {.font = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal},
                       .width = 30,
                       .height = 30,
                       .paddingH = 0,
                       .paddingV = 0,
                       .borderRadius = 4},
        .tabOpen = {.font = {.family = font::DEFAULT, .size = size::H3, .weight = QFont::Normal},
                    .width = 300,
                    .height = 50},
        .tabCreate = {.font = {.family = font::DEFAULT,
                               .size = size::H3,
                               .weight = QFont::DemiBold},
                      .width = 354,
                      .height = 50},
        .inlineBtn = {.font = {.family = font::DEFAULT,
                               .size = size::BODY,
                               .weight = QFont::Normal},
                      .paddingH = 10,
                      .paddingV = 4,
                      .borderRadius = 6},
    };
}

auto ButtonPalette::dark() -> ButtonPalette {
    return defaultButtonPalette();
}

auto ButtonPalette::light() -> ButtonPalette {
    return defaultButtonPalette();
}

auto FontPalette::defaultFontPalette() -> FontPalette {
    return FontPalette{
        .title = {.family = font::DEFAULT, .size = size::H1, .weight = QFont::DemiBold},
        .heading = {.family = font::DEFAULT, .size = size::H2, .weight = QFont::DemiBold},
        .subheading = {.family = font::DEFAULT, .size = size::H3, .weight = QFont::Normal},
        .section = {.family = font::DEFAULT, .size = size::SECTION, .weight = QFont::DemiBold},
        .inputLabel = {.family = font::DEFAULT,
                       .size = size::BODY,
                       .weight = QFont::Normal,
                       .width = resolve(Size::S)},
        .checkboxLabel = {.family = font::DEFAULT,
                          .size = size::BODY,
                          .weight = QFont::Normal,
                          .width = resolve(Size::XS)},
        .infoLabel = {.family = font::DEFAULT,
                      .size = size::BODY,
                      .weight = QFont::Normal,
                      .width = resolve(Size::S)},
        .info = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal},
        .body = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal},
        .caption = {.family = font::DEFAULT, .size = size::CAPTION, .weight = QFont::Normal},
        .mono = {.family = font::MONO, .size = size::BODY, .weight = QFont::Normal},
        .status = {.family = font::DEFAULT, .size = size::CAPTION, .weight = QFont::Normal},
    };
}

auto FontPalette::dark() -> FontPalette {
    return defaultFontPalette();
}

auto FontPalette::light() -> FontPalette {
    return defaultFontPalette();
}

auto InputPalette::defaultInputPalette() -> InputPalette {
    return InputPalette{
        .defaultInput = {.font = {.family = font::DEFAULT,
                                  .size = size::BODY,
                                  .weight = QFont::Normal}},
        .mono = {.font = {.family = font::MONO, .size = size::BODY, .weight = QFont::Normal}},
        .display = {.font = {.family = font::DEFAULT, .size = size::BODY, .weight = QFont::Normal}},
    };
}

auto InputPalette::dark() -> InputPalette {
    return defaultInputPalette();
}

auto InputPalette::light() -> InputPalette {
    return defaultInputPalette();
}

auto IconPalette::defaultIconPalette() -> IconPalette {
    return IconPalette{
        .defaultIcon = {.size = 24, .strokeWidth = 2},
    };
}

auto IconPalette::dark() -> IconPalette {
    return defaultIconPalette();
}

auto IconPalette::light() -> IconPalette {
    return defaultIconPalette();
}
