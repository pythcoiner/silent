#pragma once

#include <QLabel>
#include <QString>

struct Palette;

namespace theme {

enum class LabelRole {
    Body,          // Default body text
    Title,         // Page/screen titles
    Heading,       // Section headings
    Subheading,    // Subsection headings
    Section,       // Group/separator labels
    InputLabel,    // Labels preceding text inputs
    CheckboxLabel, // Labels for checkboxes
    InfoLabel,     // Labels preceding read-only values
    Info,          // Read-only values paired with InfoLabel
    Caption,       // Fine print, indicators
    Mono,          // Addresses, amounts, outpoints
    Status,        // Status bar messages
};

class Label : public QLabel {
    Q_OBJECT

public:
    explicit Label(const QString &text, LabelRole role = LabelRole::Body,
                   QWidget *parent = nullptr);
    explicit Label(LabelRole role = LabelRole::Body, QWidget *parent = nullptr);
    void setRole(LabelRole role);
    [[nodiscard]] auto role() const -> LabelRole;
    void setScale(qreal scale);
    [[nodiscard]] auto scale() const -> qreal;
    static auto qss(const Palette &p) -> QString;

private:
    LabelRole m_role = LabelRole::Body;
    qreal m_scale = 1.0;
    void applyRole();
};

} // namespace theme
