#pragma once

#include <QString>

struct Palette;

namespace catalog {

class Tab {
public:
    static auto qss(const Palette &p) -> QString;
};

} // namespace catalog
