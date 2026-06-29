#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <cstdint>
#include <expected>

enum class FileCode : std::uint32_t {
    file_not_found,
    file_too_large,
    file_type_unsupported,
    encoding_unsupported,
    fatal
};

struct Diagnostic {
    FileCode code {};
    std::uint32_t line {};
    std::uint32_t col {};
};

template <class T>
using Result = std::expected<T, Diagnostic>;

inline std::unexpected<Diagnostic> diagnostic_error(FileCode c, std::uint32_t line = 0, std::uint32_t col = 0) {
    return std::unexpected<Diagnostic>(std::in_place, c, line, col);
}

inline std::unexpected<Diagnostic> diagnostic_error(const Diagnostic& d) { return std::unexpected<Diagnostic>(d); }

#endif