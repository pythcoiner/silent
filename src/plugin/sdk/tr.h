#pragma once

#include <QCoreApplication>
#include <QString>

// Plugin-side translation macros.
//
// Define SILENT_PLUGIN_ID as a string literal before including this header:
//   #define SILENT_PLUGIN_ID "example"
//   #include <sdk/tr.h>
//
// The host loads plugin catalogs from:
//   ~/.silent/plugins/<id>/i18n/silent_<locale>.lang
// and expects keys to be prefixed with the same <id>. namespace.
// These macros inject that prefix at compile time so plugin call sites remain:
//   TR("my-key") / TRN("my-key", n)

#ifndef SILENT_PLUGIN_ID
// Standalone tooling (e.g. clang-tidy linting this header on its own) has no
// plugin id; fall back to a placeholder so the header still parses. Real plugin
// builds define SILENT_PLUGIN_ID before including this header.
#define SILENT_PLUGIN_ID "plugin"
#endif

#define SILENT_PLUGIN_TR_PREFIX SILENT_PLUGIN_ID "."

#define TR(id)                                                                               \
    ([&]() -> QString {                                                                      \
        return qtTrId(SILENT_PLUGIN_TR_PREFIX id);                                           \
    }())

#define TRN(id, n)                                                                           \
    ([&]() -> QString {                                                                      \
        return qtTrId(SILENT_PLUGIN_TR_PREFIX id, n);                                        \
    }())
