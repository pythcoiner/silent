#pragma once

#include <QObject>
#include <QTranslator>

class QApplication;

namespace i18n {

class I18nManager : public QObject {
    Q_OBJECT

public:
    static auto get() -> I18nManager *;

    void init(QApplication *app);
    [[nodiscard]] auto selectedLocale() const -> QString;
    [[nodiscard]] auto applyLocale(const QString &locale, bool persist = true) -> bool;

signals:
    void languageChanged(const QString &locale, bool success);

private:
    static I18nManager *s_instance; // NOLINT

    QApplication *m_app = nullptr;
    QTranslator *m_base_translator = nullptr;
    QTranslator *m_translator = nullptr;
    QString m_current_locale;
    bool m_base_translator_installed = false;
    bool m_translator_installed = false;
};

} // namespace i18n
