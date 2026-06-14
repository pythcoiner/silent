#pragma once

#include <QTabWidget>

namespace catalog {

class Tabs : public QTabWidget {
    Q_OBJECT

public:
    explicit Tabs(QWidget *parent = nullptr);
};

} // namespace catalog
