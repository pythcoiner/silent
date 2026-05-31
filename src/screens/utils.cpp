#include "utils.h"
#include "theme/Theme.h"
#include <QCoreApplication>
#include <QLocale>
#include <QPainter>
#include <QPen>
#include <Qontrol>
#include <qframe.h>

auto margin(QWidget *widget) -> QWidget * {
    return margin(widget, MARGIN);
}

auto margin(QWidget *widget, int margin) -> QWidget * {
    auto *col = (new qontrol::Column)->pushSpacer(margin)->push(widget)->pushSpacer(margin);
    auto *row = (new qontrol::Row)->pushSpacer(margin)->push(col)->pushSpacer(margin);
    return row;
}

auto frame(QWidget *widget) -> QWidget * {
    auto *f = new Frame;
    f->setFrameShape(QFrame::Box);
    f->setFrameShadow(QFrame::Sunken);
    auto *col = (new qontrol::Column)->push(widget);
    int m = 10;
    col->layout()->setContentsMargins(m, m, m, m);
    f->setLayout(col->layout());
    return f;
}

auto toBitcoin(uint64_t sats, bool with_unit) -> QString {
    double bitcoinValue = static_cast<double>(sats) / SATS;
    auto btcStr = QLocale().toString(bitcoinValue, 'f', 8);
    if (with_unit) {
        return btcStr + " BTC";
    }
    return btcStr;
}

auto shortenOutpoint(const QString &outpoint) -> QString {
    int colonIdx = outpoint.lastIndexOf(':');
    if (colonIdx < 12) {
        return outpoint;
    }
    QString txid = outpoint.left(colonIdx);
    QString vout = outpoint.mid(colonIdx);
    return txid.left(6) + "..." + txid.right(6) + vout;
}

auto coinsCount(uint64_t count) -> QString {
    auto coinsStr = QLocale().toString(static_cast<qulonglong>(count));
    return QCoreApplication::translate("screen::utils", "%1 coins").arg(coinsStr);
}

auto mapBackendErrorSummary(const QString &raw_error) -> QString {
    auto err = raw_error.trimmed().toLower();
    if (err.contains("already spent") || err.contains("double-spend") || err.contains("spent")) {
        return QCoreApplication::translate("screen::utils",
                                           "Transaction input conflict detected.");
    }
    if (err.contains("address reuse") || err.contains("reuse")) {
        return QCoreApplication::translate("screen::utils", "Address reuse detected.");
    }
    if (err.contains("timeout") || err.contains("dns") || err.contains("socket") ||
        err.contains("connect") || err.contains("connection") || err.contains("network")) {
        return QCoreApplication::translate("screen::utils",
                                           "Network connection failed. Check endpoint settings.");
    }
    if (err.contains("electrum")) {
        return QCoreApplication::translate("screen::utils", "Electrum connection failed.");
    }
    if (err.contains("blindbit") || err.contains("backend")) {
        return QCoreApplication::translate("screen::utils", "Backend request failed.");
    }
    if (err.contains("sign") || err.contains("signature")) {
        return QCoreApplication::translate("screen::utils", "Failed to sign transaction.");
    }
    if (err.contains("broadcast") || err.contains("mempool")) {
        return QCoreApplication::translate("screen::utils", "Failed to broadcast transaction.");
    }
    return QCoreApplication::translate("screen::utils", "Operation failed.");
}

auto formatBackendErrorDetails(const QString &raw_error) -> QString {
    return QCoreApplication::translate("screen::utils", "Details: %1").arg(raw_error.trimmed());
}

auto Frame::paintEvent(QPaintEvent *event) -> void {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    constexpr qreal c_pen_width = 1.5;
    QPen pen(Theme::get()->palette().border, c_pen_width);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    qreal half = c_pen_width / 2.0;
    QRectF rect = QRectF(this->rect()).adjusted(half, half, -half, -half);
    painter.drawRoundedRect(rect, 8, 8);
}
