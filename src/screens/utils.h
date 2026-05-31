#pragma once

#include "theme/Palette.h"
#include <QFrame>
#include <cstdint>
#include <qtmetamacros.h>
#include <qwidget.h>

// Layout constants — aliases from design system
const int MARGIN = resolve(Padding::XL);
const int LEFT_MARGIN = MARGIN;
const int TOP_MARGIN = MARGIN;
const int RIGHT_MARGIN = MARGIN;
const int BOTTOM_MARGIN = MARGIN;

const int LABEL_WIDTH = resolve(Size::S);
const int INPUT_WIDTH = resolve(Size::M);
const int PRICE_WIDTH = resolve(Size::M);

const int V_SPACER = resolve(Spacing::XS);
const int H_SPACER = resolve(Spacing::XS);

const int SATS = 100000000;

auto margin(QWidget *widget) -> QWidget *;
auto margin(QWidget *widget, int margin) -> QWidget *;
auto frame(QWidget *widget) -> QWidget *;
auto toBitcoin(uint64_t sats, bool with_unit = true) -> QString;
auto shortenOutpoint(const QString &outpoint) -> QString;
auto coinsCount(uint64_t count) -> QString;
auto mapBackendErrorSummary(const QString &raw_error) -> QString;
auto formatBackendErrorDetails(const QString &raw_error) -> QString;

class Frame : public QFrame {
    Q_OBJECT
public:
    Frame(QWidget *parent = nullptr) : QFrame(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    auto paintEvent(QPaintEvent *event) -> void override;
};
