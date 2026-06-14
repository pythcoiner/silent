#pragma once

#include "catalog/inputs/Input.h"
#include "catalog/form/ValidationMark.h"
#include <QWidget>

namespace catalog {

class ValidatedInput : public QWidget {
    Q_OBJECT

public:
    explicit ValidatedInput(InputRole role = InputRole::Default, QWidget *parent = nullptr);
    [[nodiscard]] auto input() const -> Input *;
    void setWidth(Size s);
    void setState(ValidationMark::State state);

signals:
    void textChanged(const QString &text);

public slots:
    void setValid(bool valid);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Input *m_input = nullptr;
    ValidationMark *m_mark = nullptr;
};

} // namespace catalog
