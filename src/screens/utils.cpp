#include "common.h"
#include <Qontrol>
#include <QPainter>
#include <QPen>
#include <qboxlayout.h>
#include <qframe.h>
#include <qnamespace.h>

auto margin(QWidget *widget) -> QWidget * {
    return margin(widget, MARGIN);
}

auto margin(QWidget *widget, int margin) -> QWidget * {
    auto *col = (new qontrol::Column)
                    ->pushSpacer(margin)
                    ->push(widget)
                    ->pushSpacer(margin);
    auto *row = (new qontrol::Row)
                    ->pushSpacer(margin)
                    ->push(col)
                    ->pushSpacer(margin);
    return row;
}

auto frame(QWidget *widget) -> QWidget * {
    auto *frame = new Frame;
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Sunken);
    auto *layout = new QVBoxLayout(frame);
    layout->addWidget(widget);
    widget->setParent(frame);
    int m = 10;
    layout->setContentsMargins(m, m, m, m);
    return frame;
}

auto toBitcoin(uint64_t sats, bool with_unit) -> QString {
    double bitcoinValue = static_cast<double>(sats) / SATS;
    auto btcStr = QString::number(bitcoinValue);
    if (with_unit) {
        return btcStr + " BTC";
    }
    return btcStr;
}

auto coinsCount(uint64_t count) -> QString {
    auto coinsStr = QString::number(count);
    return coinsStr + " coins";
}

void Frame::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(Qt::darkGray, 3);
    painter.setPen(pen);

    QRectF rect = this->rect();
    painter.drawRoundedRect(rect, 10, 10);
}
