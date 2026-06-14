#pragma once

#include "catalog/Table.h"
#include <cstdint>

namespace catalog {

// The History panel's transaction table: owns the column layout and turns a
// raw transaction (direction / txid / height / amount) into a formatted row.
class HistoryTable : public Table {
public:
    explicit HistoryTable(QWidget *parent = nullptr);
    void addEntry(const QString &direction, const QString &txid, uint32_t height, uint64_t amount);
};

} // namespace catalog
