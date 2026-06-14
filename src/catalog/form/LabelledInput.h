#pragma once

#include <QString>
#include <QWidget>

struct Palette;

namespace catalog {

class Input;
class ValidationMark;

// A label stacked on top of an input, with an optional trailing validation mark
// alongside the input. When no mark is given, an equal-width slot is still
// reserved so inputs across sibling rows keep the same length.
class LabelledInput : public QWidget {
    Q_OBJECT

public:
    LabelledInput(const QString &label, Input *input, ValidationMark *mark = nullptr,
                  QWidget *parent = nullptr);
    static auto qss(const Palette &p) -> QString;
};

} // namespace catalog
