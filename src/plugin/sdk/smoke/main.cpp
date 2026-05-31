#include <sdk/types.h>
#include <sdk/interfaces/module.h>
#include <sdk/interfaces/account.h>
#include <sdk/interfaces/feed.h>
#include <sdk/interfaces/signer.h>
#include <sdk/interfaces/theme.h>
#include <sdk/host.h>

#define SILENT_PLUGIN_ID "smoke-plugin"
#include <sdk/tr.h>

int main() {
    plugin::Coin coin{
        .outpoint = "", .value = 0, .height = 0, .label = "", .spent = false, .accountType = ""};
    plugin::Balance balance{.confirmed = 0, .unconfirmed = 0};
    IModule *module = nullptr;
    IPlugin *plugin = nullptr;
    IAccount *account = nullptr;
    IFeed *feed = nullptr;
    ISigner *signer = nullptr;
    IThemeProvider *themeProvider = nullptr;
    Host *host = nullptr;
    HostEvents *hostEvents = nullptr;
    QString translated = TR("hello");
    (void)coin;
    (void)balance;
    (void)module;
    (void)plugin;
    (void)account;
    (void)feed;
    (void)signer;
    (void)themeProvider;
    (void)host;
    (void)hostEvents;
    (void)translated;
    return 0;
}
