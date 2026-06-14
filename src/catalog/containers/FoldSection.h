#pragma once

#include <QPointer>
#include <QWidget>

struct Palette;

namespace qontrol {
class Column;
}

namespace catalog {

class Button;

class FoldSection : public QWidget {
    Q_OBJECT

public:
    explicit FoldSection(const QString &title, QWidget *parent = nullptr);
    void setTitle(const QString &title);
    void setContent(QWidget *content);
    void detachContent();
    void setExpanded(bool expanded);
    [[nodiscard]] auto isExpanded() const -> bool;
    static auto qss(const Palette &p) -> QString;

signals:
    void toggled(bool expanded);

public slots:
    void onHeaderClicked();

protected:
    void updateHeader();

private:
    QString m_title;
    qontrol::Column *m_column = nullptr;
    Button *m_header = nullptr;
    QPointer<QWidget> m_content;
    bool m_expanded = true;
};

} // namespace catalog
