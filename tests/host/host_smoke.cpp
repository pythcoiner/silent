// Opt-in plugin host/registry smoke test.
//
// Replaces the removed runtime env-var smoke (runDevTabSmokeIfRequested) with a
// headless lifecycle exercise driven by MOCK plugins. It never touches
// MainWindow or real tabs: it only drives Host::registerInstance/instances/
// removeInstance and the PluginRegistry builtin path.
//
// Built only when SILENT_BUILD_HOST_SMOKE=ON. Run under QT_QPA_PLATFORM=offscreen
// and a throwaway HOME so the Rust app-state FFI (~/.silent/app.json) stays
// hermetic. Exits 0 on success, non-zero on the first failed check.

#include <cstdio>

#include <QApplication>
#include <QList>
#include <QString>

#include <Qontrol>
#include <common.h>

#include <host/Host.h>
#include <host/PluginRegistry.h>
#include <interfaces/instance.h>
#include <interfaces/module.h>

#include "MockPlugin.h"

namespace {

bool g_ok = true;

void check(bool cond, const char *msg) {
    if (!cond) {
        g_ok = false;
        std::fprintf(stderr, "FAIL: %s\n", msg);
    }
}

auto hostInstanceCount() -> int {
    return Host::get()->instances().size();
}

auto hostHasInstance(const QString &id) -> bool {
    for (auto *instance : Host::get()->instances()) {
        if (instance != nullptr && instance->id() == id) {
            return true;
        }
    }
    return false;
}

} // namespace

auto main(int argc, char *argv[]) -> int {
    QApplication app(argc, argv);

    // Initialise the host singleton before the registry connects to its events.
    Host::get();

    smoke::MockPlugin plugin;
    auto *module = plugin.module();

    PluginRegistry registry;
    smoke::SignalSpy spy;
    QObject::connect(&registry, &PluginRegistry::enabledPluginsChanged, &spy,
                     &smoke::SignalSpy::onSignal, qontrol::UNIQUE);

    int baselineInstances = hostInstanceCount();

    // ===== registerBuiltin: enabled + builtin =====
    registry.registerBuiltin(&plugin);
    check(registry.isPluginEnabled(QStringLiteral("mock.plugin")),
          "mock plugin should be enabled after registerBuiltin");
    check(registry.isPluginBuiltin(QStringLiteral("mock.plugin")),
          "mock plugin should be marked builtin");

    // ===== enabledModulesForLauncher contains the module =====
    check(registry.enabledModulesForLauncher().contains(module),
          "enabledModulesForLauncher should contain the mock module");

    // setEnabled(false) on a builtin is a no-op by design (guarded by
    // pluginHasBuiltinModules); it must stay enabled.
    registry.setEnabled(QStringLiteral("mock.plugin"), false);
    check(registry.isPluginEnabled(QStringLiteral("mock.plugin")),
          "builtin plugin must remain enabled (setEnabled false is a no-op)");

    // Deliver the module's pending list()/autoStart() responses now that the
    // registry has recorded the pending requests during registration.
    module->flush();

    // ===== launcher instances populated via the list() response =====
    auto launcherInstances = registry.launcherInstancesForModule(module);
    bool hasInst1 = false;
    for (const auto &entry : launcherInstances) {
        if (entry.first == QStringLiteral("inst-1")) {
            hasInst1 = true;
            break;
        }
    }
    check(hasInst1, "launcherInstancesForModule should list inst-1");
    check(spy.count() > 0, "enabledPluginsChanged should have fired during registration");

    int afterListCount = spy.count();

    // ===== startLauncherInstance: host grows, owner tracked =====
    bool started = registry.startLauncherInstance(module, QStringLiteral("inst-1"));
    check(started, "startLauncherInstance should succeed");
    check(hostInstanceCount() == baselineInstances + 1,
          "host instances should grow by one after start");
    check(hostHasInstance(QStringLiteral("inst-1")),
          "host should hold the started instance inst-1");

    auto *mockInstance = module->liveInstance(QStringLiteral("inst-1"));
    check(mockInstance != nullptr, "module should track the live mock instance");

    // ===== deleteInstance: dropped, stopped, no leak =====
    registry.deleteInstance(module, QStringLiteral("inst-1"));
    check(hostInstanceCount() == baselineInstances,
          "host instance count should return to baseline after delete");
    check(!hostHasInstance(QStringLiteral("inst-1")),
          "host should no longer hold inst-1 after delete");
    check(mockInstance != nullptr && mockInstance->wasStopped(),
          "instance stop() should have been called on delete");
    check(module->deletedIds().contains(QStringLiteral("inst-1")),
          "owning module deleteInstance should have been invoked");
    check(spy.count() > afterListCount,
          "enabledPluginsChanged should fire again on deleteInstance");

    if (g_ok) {
        std::fprintf(stdout, "host smoke: OK\n");
        return 0;
    }
    std::fprintf(stderr, "host smoke: FAILED\n");
    return 1;
}
