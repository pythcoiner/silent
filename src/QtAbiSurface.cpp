#include <QGuiApplication>
#include <QObject>
#include <QSvgRenderer>
#include <QWidget>
#include <Qontrol>

namespace {

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(gnu::used)
#define SILENT_USED [[gnu::used]]
#endif
#endif

#ifndef SILENT_USED
#if defined(__GNUC__) || defined(__clang__)
#define SILENT_USED __attribute__((used))
#else
#define SILENT_USED
#endif
#endif

// Keep concrete Qt/qontrol ABI symbols alive so --export-dynamic can publish
// the host-supported plugin surface even with -O2/--gc-sections.
SILENT_USED const void *const ANCHORS[] = {
    static_cast<const void *>(&QObject::staticMetaObject),
    static_cast<const void *>(&QGuiApplication::staticMetaObject),
    static_cast<const void *>(&QWidget::staticMetaObject),
    static_cast<const void *>(&QSvgRenderer::staticMetaObject),
    static_cast<const void *>(&qontrol::Window::staticMetaObject),
    static_cast<const void *>(&qontrol::Screen::staticMetaObject),
    static_cast<const void *>(&qontrol::Modal::staticMetaObject),
    static_cast<const void *>(&qontrol::Row::staticMetaObject),
    static_cast<const void *>(&qontrol::Column::staticMetaObject),
    static_cast<const void *>(&qontrol::Panel::staticMetaObject),
    static_cast<const void *>(&qontrol::widgets::ToggleSwitch::staticMetaObject),
};

#undef SILENT_USED

} // namespace
