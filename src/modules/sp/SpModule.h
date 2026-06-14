#pragma once

#include "interfaces/instance.h"
#include "interfaces/module.h"

#include <memory>
#include <vector>

class SpModule final : public IModule {
    Q_OBJECT

public:
    SpModule();

    [[nodiscard]] QMap<QString, QString> meta() const override;
    ReqId list() override;
    ReqId autoStart() override;
    void startInstance(const QString &id) override;

private:
    ReqId nextReqId();

    std::vector<std::unique_ptr<IInstance>> m_instances;
    ReqId m_next_req_id = 0;
};
