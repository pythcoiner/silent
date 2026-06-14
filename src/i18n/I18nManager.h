#pragma once

#include <QObject>
#include <QTranslator>
#include <QHash>

class QApplication;

namespace i18n {

class I18nManager : public QObject {
    Q_OBJECT

public:
    static auto get() -> I18nManager *;

    void init(QApplication *app);
    [[nodiscard]] auto selectedLocale() const -> QString;
    [[nodiscard]] auto applyLocale(const QString &locale, bool persist = true) -> bool;
    auto registerPluginTranslations(const QString &plugin_id, const QString &dir) -> bool;
    void unregisterPluginTranslations(const QString &plugin_id);

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

    struct PluginTranslatorEntry {
        QTranslator *translator = nullptr;
        QString plugin_id;
        QString dir;
        bool installed = false;
    };
    QHash<QString, PluginTranslatorEntry> m_plugin_translators;
};

} // namespace i18n
