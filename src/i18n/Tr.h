#pragma once

#include <QCoreApplication>
#include <QString>
#include <QtGlobal>

namespace i18n::detail {

template <size_t N>
consteval auto isValidId(const char (&id)[N]) -> bool {
    if constexpr (N <= 1) {
        return false;
    }

    for (size_t i = 0; i + 1 < N; ++i) {
        auto cCh = static_cast<unsigned char>(id[i]);
        bool isLower = (cCh >= 'a' && cCh <= 'z');
        bool isDigit = (cCh >= '0' && cCh <= '9');
        bool isDash = (cCh == '-');
        if (!(isLower || isDigit || isDash)) {
            return false;
        }
    }
    return true;
}

} // namespace i18n::detail

#define I18N_STRINGIFY_INNER(x) #x
#define I18N_STRINGIFY(x) I18N_STRINGIFY_INNER(x)

#ifdef I18N_TRACE_IDS
// GCC and Clang both support #pragma message; parse lines containing I18N_ID:.
#define I18N_TRACE_ID(id) _Pragma(I18N_STRINGIFY(message("I18N_ID:" id)))
#define I18N_TRACE_IDN(id) _Pragma(I18N_STRINGIFY(message("I18N_IDN:" id)))
#else
#define I18N_TRACE_ID(id)
#define I18N_TRACE_IDN(id)
#endif

#define I18N_VALIDATE_ID(id)                                                                 \
    static_assert(::i18n::detail::isValidId(id),                                             \
                  "Invalid i18n id: use lowercase letters, digits, and hyphen only")

#define TR(id)                                                                               \
    ([&]() -> QString {                                                                      \
        I18N_VALIDATE_ID(id);                                                                \
        I18N_TRACE_ID(id);                                                                   \
        return qtTrId(id);                                                                   \
    }())

#define TRN(id, n)                                                                           \
    ([&]() -> QString {                                                                      \
        I18N_VALIDATE_ID(id);                                                                \
        I18N_TRACE_IDN(id);                                                                  \
        return qtTrId(id, n);                                                                \
    }())
