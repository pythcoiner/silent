#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

#include <host/Host.h>
#include <interfaces/account.h>
#include <interfaces/feed.h>
#include <interfaces/instance.h>
#include <interfaces/module.h>
#include <interfaces/signer.h>
#include <interfaces/theme.h>
#include <interfaces/types.h>

namespace smoke {

// Minimal IInstance used by the host smoke test. The smoke never dereferences
// the account/feed/signer/theme providers, so they return nullptr by contract.
class MockInstance final : public QObject, public IInstance {
    Q_OBJECT
    Q_INTERFACES(IInstance)

public:
    explicit MockInstance(QString id, QObject *parent = nullptr)
        : QObject(parent), m_id(std::move(id)) {}
    ~MockInstance() override = default;

    [[nodiscard]] QString id() const override {
        return m_id;
    }

    void stop() override {
        m_stopped = true;
    }

    auto account() -> IAccount * override {
        return nullptr;
    }

    auto feed() -> IFeed * override {
        return nullptr;
    }

    auto signer() -> ISigner * override {
        return nullptr;
    }

    auto theme() -> IThemeProvider * override {
        return nullptr;
    }

    [[nodiscard]] auto wasStopped() const -> bool {
        return m_stopped;
    }

private:
    QString m_id;
    bool m_stopped = false;
};

// Mock module modelling the real async modules: list()/autoStart() return a
// request id without emitting, then flush() delivers the responses. This lets
// the registry register the pending request before the signal arrives, so the
// response is matched with the module as sender(). startInstance() registers a
// MockInstance via the host.
class MockModule final : public IModule {
    Q_OBJECT

public:
    explicit MockModule(QObject *parent = nullptr) : IModule(parent) {}
    ~MockModule() override = default;

    [[nodiscard]] QMap<QString, QString> meta() const override {
        QMap<QString, QString> meta;
        meta.insert(QStringLiteral("id"), QStringLiteral("mock.module"));
        meta.insert(QStringLiteral("name"), QStringLiteral("Mock Module"));
        return meta;
    }

    auto list() -> ReqId override {
        m_list_req_id = ++m_next_req_id;
        return m_list_req_id;
    }

    auto autoStart() -> ReqId override {
        m_auto_start_req_id = ++m_next_req_id;
        return m_auto_start_req_id;
    }

    // Deliver the pending list()/autoStart() responses. Emitted from the module
    // itself so the registry sees sender() == this module.
    void flush() {
        if (m_auto_start_req_id != 0) {
            ReqId req_id = m_auto_start_req_id;
            m_auto_start_req_id = 0;
            emit IModule::autoStart(req_id, QStringList{});
        }
        if (m_list_req_id != 0) {
            ReqId req_id = m_list_req_id;
            m_list_req_id = 0;
            QList<QPair<QString, QString>> instances;
            instances.append({QStringLiteral("inst-1"), QStringLiteral("Instance One")});
            emit IModule::instances(req_id, instances);
        }
    }

    void startInstance(const QString &id) override {
        auto *instance = new MockInstance(id.trimmed(), this);
        m_live_instances.append(instance);
        Host::get()->registerInstance(id.trimmed(), instance);
    }

    void deleteInstance(const QString &id) override {
        m_deleted_ids.append(id.trimmed());
    }

    [[nodiscard]] auto deletedIds() const -> QStringList {
        return m_deleted_ids;
    }

    [[nodiscard]] auto liveInstance(const QString &id) const -> MockInstance * {
        for (auto *instance : m_live_instances) {
            if (instance != nullptr && instance->id() == id.trimmed()) {
                return instance;
            }
        }
        return nullptr;
    }

private:
    ReqId m_next_req_id = 0;
    ReqId m_list_req_id = 0;
    ReqId m_auto_start_req_id = 0;
    QList<MockInstance *> m_live_instances;
    QStringList m_deleted_ids;
};

// Counts emissions of a signal via a named slot (no lambdas in connect()).
class SignalSpy final : public QObject {
    Q_OBJECT

public:
    explicit SignalSpy(QObject *parent = nullptr) : QObject(parent) {}
    ~SignalSpy() override = default;

    [[nodiscard]] auto count() const -> int {
        return m_count;
    }

public slots:
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void onSignal() {
        ++m_count;
    }

private:
    int m_count = 0;
};

// Builtin-style plugin exposing the single mock module.
class MockPlugin final : public QObject, public IPlugin {
    Q_OBJECT
    Q_INTERFACES(IPlugin)

public:
    explicit MockPlugin(QObject *parent = nullptr)
        : QObject(parent), m_module(new MockModule(this)) {}
    ~MockPlugin() override = default;

    [[nodiscard]] QMap<QString, QString> meta() const override {
        QMap<QString, QString> meta;
        meta.insert(QStringLiteral("id"), QStringLiteral("mock.plugin"));
        meta.insert(QStringLiteral("name"), QStringLiteral("Mock Plugin"));
        return meta;
    }

    [[nodiscard]] QList<IModule *> modules() const override {
        return {m_module};
    }

    [[nodiscard]] auto module() const -> MockModule * {
        return m_module;
    }

private:
    MockModule *m_module = nullptr;
};

} // namespace smoke
