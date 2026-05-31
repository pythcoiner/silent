#pragma once

#include <optional>

#include <QByteArray>
#include <QObject>
#include <QString>

#include <sdk/types.h>

class IFeed : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    ~IFeed() override = default;

    [[nodiscard]] virtual bool implemented() const = 0;
    virtual ReqId raw(const QByteArray &request) = 0;

signals:
    void feeRate(ReqId req_id, double satvb);
    void conversionRate(ReqId req_id, QString currency, double value);
    void raw(ReqId req_id, QByteArray response);
    void error(std::optional<ReqId> req_id, QString message);
};

Q_DECLARE_INTERFACE(IFeed, "dev.silent.IFeed/1.0")
