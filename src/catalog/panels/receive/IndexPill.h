#pragma once

#include <QWidget>

struct Palette;

namespace catalog {

class Label;

// A small clickable pill showing a QR icon + a derivation index in monospace
// (design Widgets.jsx IndexPill). Toggling `active` swaps its background to
// signal that its QR is expanded.
class IndexPill : public QWidget {
    Q_OBJECT

public:
    explicit IndexPill(QWidget *parent = nullptr);
    void setIndex(int index);
    void setActive(bool active);
    static auto qss(const Palette &p) -> QString;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setStateProperty(const char *name, bool value);

private:
    Label *m_index = nullptr;
};

} // namespace catalog
