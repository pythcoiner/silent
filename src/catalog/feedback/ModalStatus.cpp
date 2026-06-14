#include "catalog/feedback/ModalStatus.h"

#include "theme/Icon.h"
#include "catalog/display/Label.h"
#include "theme/Palette.h"

#include <Qontrol>

namespace catalog {

ModalStatus::ModalStatus(QWidget *parent) : QWidget(parent) {
    m_icon = new Label(LabelRole::Title, this);
    m_icon->setAlignment(Qt::AlignCenter);
    m_icon->setFixedHeight(metric::MODAL_STATUS_ICON_SIZE);
    m_title = new Label("Review", LabelRole::Section, this);
    auto titleFont = m_title->font();
    titleFont.setWeight(QFont::Medium); // design title is weight 500
    m_title->setFont(titleFont);
    m_title->setAlignment(Qt::AlignCenter);
    m_subtitle = new Label(LabelRole::Caption, this); // caption gives the muted color
    auto subFont = m_subtitle->font();
    subFont.setPointSize(size::BODY); // design subtitle is body size, muted
    m_subtitle->setFont(subFont);
    m_subtitle->setWordWrap(true);
    m_subtitle->setAlignment(Qt::AlignCenter);

    (new qontrol::Column)
        ->spacing(resolve(Spacing::M) - resolve(Spacing::XS))
        ->push(m_icon)
        ->push(m_title)
        ->push(m_subtitle)
        ->into(this);

    setState(State::Review);
}

void ModalStatus::setState(State state) {
    int size = metric::MODAL_STATUS_ICON_SIZE;
    switch (state) {
    case State::Review:
    case State::Signing:
    case State::Broadcasting:
        m_icon->setPixmap(icon::refresh(IconColor::Muted).pixmap(size, size));
        break;
    case State::Success:
        m_icon->setPixmap(icon::check(IconColor::Success).pixmap(size, size));
        break;
    case State::Error:
        m_icon->setPixmap(icon::x(IconColor::Error).pixmap(size, size));
        break;
    }

    switch (state) {
    case State::Review:
        m_title->setText("Review");
        break;
    case State::Signing:
        m_title->setText("Signing");
        break;
    case State::Broadcasting:
        m_title->setText("Broadcasting");
        break;
    case State::Success:
        m_title->setText("Success");
        break;
    case State::Error:
        m_title->setText("Error");
        break;
    }
}

void ModalStatus::setSubtitle(const QString &subtitle) {
    m_subtitle->setText(subtitle);
}

} // namespace catalog
