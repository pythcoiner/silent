#pragma once

#include "Palette.h"
#include <QObject>

enum class ThemeMode { Light, Dark };

class Theme : public QObject {
    Q_OBJECT

public:
    static void init();
    static auto get() -> Theme *;
    void setMode(ThemeMode mode);
    void setPalette(const Palette &palette);
    [[nodiscard]] auto mode() const -> ThemeMode;
    [[nodiscard]] auto palette() const -> const Palette &;
    [[nodiscard]] auto fontPalette() const -> const FontPalette &;
    [[nodiscard]] auto buttonPalette() const -> const ButtonPalette &;
    [[nodiscard]] auto iconPalette() const -> const IconPalette &;
    [[nodiscard]] auto inputPalette() const -> const InputPalette &;

    void apply();

signals:
    void themeChanged(ThemeMode mode);

private:
    explicit Theme(QObject *parent = nullptr);
    static Theme *s_instance; // NOLINT(readability-identifier-naming)
    ThemeMode m_mode = ThemeMode::Dark;
    Palette m_palette = Palette::dark();
    FontPalette m_font_palette = FontPalette::dark();
    ButtonPalette m_button_palette = ButtonPalette::dark();
    IconPalette m_icon_palette = IconPalette::dark();
    InputPalette m_input_palette = InputPalette::dark();
};
