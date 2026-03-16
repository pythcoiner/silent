#pragma once

#include <QString>

struct Palette;

namespace theme {

class Tab {
public:
    static auto qss(const Palette &p) -> QString;
};

} // namespace theme
