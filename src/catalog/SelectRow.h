#pragma once

#include "theme/Palette.h"
#include <QIcon>
#include <QWidget>

namespace catalog {

class Label;

class SelectRow : public QWidget {
    Q_OBJECT

public:
    explicit SelectRow(QWidget *parent = nullptr);
    void setIcon(const QIcon &icon);
    void setTitle(const QString &title);
    void setMetadata(const QString &metadata);
    static auto qss(const Palette &p) -> QString;

signals:
    void clicked();

protected:
    [[nodiscard]] auto eventFilter(QObject *watched, QEvent *event) -> bool override;

private:
    Label *m_icon = nullptr;
    Label *m_title = nullptr;
    Label *m_metadata = nullptr;
    Label *m_chevron = nullptr;
};

} // namespace catalog
