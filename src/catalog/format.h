#pragma once

#include <QString>
#include <cstdint>

// Shared display formatters usable by both catalog components and views.

const int SATS = 100000000;

auto toBitcoin(uint64_t sats, bool with_unit = true) -> QString;
