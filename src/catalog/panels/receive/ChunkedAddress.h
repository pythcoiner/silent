#pragma once

#include <QLabel>
#include <QString>

namespace catalog {

// An address rendered as 4-char monospace groups with alternating dim/full
// coloring, centered, on one or two lines (design Address.jsx ChunkedAddress).
// Clicking toggles between the chunked view and one unbroken string.
class ChunkedAddress : public QLabel {
    Q_OBJECT

public:
    // Auto   : two lines only for long addresses (> 14 chunks), else one.
    // Single : always one line.
    // Double : two lines (when there is more than one chunk).
    enum class LineMode { Auto, Single, Double };

    explicit ChunkedAddress(QWidget *parent = nullptr);
    void setAddress(const QString &address);
    [[nodiscard]] auto address() const -> QString;
    void setLineMode(LineMode mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void rebuild();

    QString m_address;
    LineMode m_mode = LineMode::Auto;
    bool m_chunked = true;
};

} // namespace catalog
