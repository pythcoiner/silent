#pragma once

#include <QWidget>

namespace catalog {

class QrCode : public QWidget {
    Q_OBJECT

public:
    // Real QR encoding is deferred; this keeps the widget API stable.
    explicit QrCode(QWidget *parent = nullptr);
    void setData(const QString &data);
    [[nodiscard]] auto data() const -> QString;

protected:
    void paintEvent(QPaintEvent *event) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;

private:
    QString m_data;
};

} // namespace catalog
