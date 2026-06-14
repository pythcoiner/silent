#pragma once

#include "theme/Palette.h"
#include <QWidget>
#include <cstdint>

namespace catalog {

class Label;

class BalanceHeader : public QWidget {
    Q_OBJECT

public:
    explicit BalanceHeader(QWidget *parent = nullptr);
    void setConfirmed(const QString &amount);
    void setUnconfirmed(uint64_t sats);
    void setCoinCount(int count);
    static auto qss(const Palette &p) -> QString;

private:
    Label *m_title = nullptr;
    Label *m_confirmed = nullptr;
    Label *m_unit = nullptr;
    Label *m_coin_count = nullptr;
    QWidget *m_unconfirmed_row = nullptr;
    Label *m_unconfirmed = nullptr;
};

} // namespace catalog
