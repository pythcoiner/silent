# Embedded Resources

Fonts and other binary assets are embedded as C++ headers containing `constexpr`
byte arrays. This avoids the Qt resource system while keeping assets compiled
into the binary.

## Embedding a font

```bash
cd src/resources
./embed_font.sh /path/to/Font-Regular.ttf font/font_name.h FONT_NAME
```

This generates a header with a `constexpr uint8_t` array in the
`embedded_font` namespace.

## Loading at runtime

```cpp
#include "resources/font/noto_sans.h"
#include <QFontDatabase>

QFontDatabase::addApplicationFontFromData(
    QByteArray(reinterpret_cast<const char *>(embedded_font::NOTO_SANS),
               embedded_font::NOTO_SANS_SIZE));
```

Call this once before creating any widgets (e.g. in `main.cpp` after
`QApplication` is constructed).

## Current embedded fonts

| Header | Font | Variable |
|--------|------|----------|
| `font/noto_sans.h` | Noto Sans Regular | `NOTO_SANS` |

## Embedding icons (Lucide)

```bash
cd src/resources
./fetch_icons.sh
```

This clones the [Lucide](https://lucide.dev) icon set, generates one header per
SVG in `icon/<name>.h` (raw string literal in the `embedded_icon` namespace),
and a master `icon/icons.h` that includes all of them.

Only icons that are `#include`d in C++ code contribute to binary size.

### Using an icon

1. Include the generated header:
   ```cpp
   #include "resources/icon/trash_2.h"
   ```
2. Render with `renderIcon()` from `theme/Icon.h`:
   ```cpp
   auto qicon = renderIcon(embedded_icon::TRASH_2, 24, color, 2);
   ```
3. Or use a named helper (add new ones in `theme/Icon.cpp`):
   ```cpp
   button->setIcon(icon::trash());
   ```

Named helpers read color from `Palette` and size/stroke from `IconPalette`
so icons automatically follow the current theme.

### Browsing available icons

Browse and search the full icon set at **https://lucide.dev/icons**.
Use the search bar to find icons by name or concept (e.g. "wallet",
"arrow", "settings"). The filename on the site maps directly to the
header name: `arrow-right` on the site → `icon/arrow_right.h` →
`embedded_icon::ARROW_RIGHT`.
