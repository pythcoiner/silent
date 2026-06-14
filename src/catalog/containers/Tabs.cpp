#include "catalog/containers/Tabs.h"

namespace catalog {

Tabs::Tabs(QWidget *parent) : QTabWidget(parent) {
    setProperty("class", "tabs");
}

} // namespace catalog
