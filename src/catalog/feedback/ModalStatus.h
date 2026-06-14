#pragma once

#include <QWidget>

namespace catalog {

class Label;

class ModalStatus : public QWidget {
    Q_OBJECT

public:
    enum class State { Review, Signing, Broadcasting, Success, Error };

    explicit ModalStatus(QWidget *parent = nullptr);
    void setState(State state);
    void setSubtitle(const QString &subtitle);

private:
    Label *m_icon = nullptr;
    Label *m_title = nullptr;
    Label *m_subtitle = nullptr;
};

} // namespace catalog
