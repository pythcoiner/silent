#pragma once

#include "SpModule.h"
#include "interfaces/module.h"

class SpPlugin final : public IPlugin {
public:
    QMap<QString, QString> meta() const override;
    QList<IModule *> modules() const override;

private:
    mutable SpModule m_module;
};
