#pragma once

#include <QString>

#include <interfaces/account.h>
#include <interfaces/feed.h>
#include <interfaces/signer.h>
#include <interfaces/theme.h>

class IInstance {
public:
    virtual ~IInstance() = default;

    [[nodiscard]] virtual QString id() const = 0;
    virtual void stop() = 0;

    virtual IAccount *account() = 0;
    virtual IFeed *feed() = 0;
    virtual ISigner *signer() = 0;
    virtual IThemeProvider *theme() = 0;
};

Q_DECLARE_INTERFACE(IInstance, "dev.silent.IInstance/1.0")
