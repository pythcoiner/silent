#pragma once

#include "theme/Palette.h"
#include <QLabel>

namespace catalog {

class ValidationMark : public QLabel {
    Q_OBJECT

public:
    enum class State { None, Valid, Invalid };

    explicit ValidationMark(QWidget *parent = nullptr);
    void setState(State state);
    [[nodiscard]] auto state() const -> State;
    static auto qss(const Palette &p) -> QString;

private:
    void applyState();

    State m_state = State::None;
};

} // namespace catalog
