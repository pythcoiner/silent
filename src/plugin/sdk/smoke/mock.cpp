#include <QMap>
#include <QString>
#include <utility>

#include <sdk/interfaces/account.h>
#include <sdk/interfaces/feed.h>
#include <sdk/interfaces/instance.h>
#include <sdk/interfaces/signer.h>
#include <sdk/interfaces/theme.h>

namespace {
ReqId nextReqId() {
    static ReqId reqId = 0;
    return ++reqId;
}

class MockAccount : public IAccount {
public:
    using IAccount::IAccount;

    [[nodiscard]] bool implemented() const override { return true; }
    ReqId name() override { return nextReqId(); }
    ReqId info() override { return nextReqId(); }
    ReqId coins() override { return nextReqId(); }
    ReqId balance() override { return nextReqId(); }
    ReqId newReceiveAddress() override { return nextReqId(); }
    ReqId newChangeAddress() override { return nextReqId(); }
    ReqId raw(const QByteArray & /*request*/) override { return nextReqId(); }
};

class MockFeed : public IFeed {
public:
    using IFeed::IFeed;

    [[nodiscard]] bool implemented() const override { return true; }
    ReqId raw(const QByteArray & /*request*/) override { return nextReqId(); }
};

class MockSigner : public ISigner {
public:
    using ISigner::ISigner;

    [[nodiscard]] bool implemented() const override { return true; }
    [[nodiscard]] QString id() const override { return QStringLiteral("mock-signer"); }
    [[nodiscard]] QString fingerprint() const override { return QStringLiteral("00000000"); }
    [[nodiscard]] QString walletName() const override { return QStringLiteral("Mock Wallet"); }
    ReqId init() override { return nextReqId(); }
    ReqId info() override { return nextReqId(); }
    ReqId getXpub(const QString & /*path*/) override { return nextReqId(); }
    ReqId isDescriptorRegistered(const QString & /*descriptor*/) override { return nextReqId(); }
    ReqId registerDescriptor(const QString & /*descriptor*/) override { return nextReqId(); }
    ReqId signWithDescriptor(const QString & /*psbt*/, const QString & /*descriptor*/) override {
        return nextReqId();
    }
    ReqId raw(const QByteArray & /*request*/) override { return nextReqId(); }
};

class MockThemeProvider : public IThemeProvider {
public:
    using IThemeProvider::IThemeProvider;

    [[nodiscard]] bool implemented() const override { return true; }
    ReqId themes() override { return nextReqId(); }
    ReqId palette(const QString & /*name*/) override { return nextReqId(); }
    ReqId raw(const QByteArray & /*request*/) override { return nextReqId(); }
};

class MockInstance : public IInstance {
public:
    explicit MockInstance(QString instance_id) : m_instance_id(std::move(instance_id)) {}

    [[nodiscard]] QString id() const override { return m_instance_id; }
    void stop() override {}

    IAccount *account() override { return &m_account; }
    IFeed *feed() override { return &m_feed; }
    ISigner *signer() override { return &m_signer; }
    IThemeProvider *theme() override { return &m_theme; }

private:
    QString m_instance_id;
    MockAccount m_account;
    MockFeed m_feed;
    MockSigner m_signer;
    MockThemeProvider m_theme;
};
} // namespace

int main() {
    MockInstance instance(QStringLiteral("mock-instance"));
    (void)instance.account()->implemented();
    (void)instance.feed()->implemented();
    (void)instance.signer()->implemented();
    (void)instance.theme()->implemented();
    return 0;
}
