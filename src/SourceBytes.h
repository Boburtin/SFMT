#ifndef SOURCEBYTES_H
#define SOURCEBYTES_H

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "Diagnostic.h"

class SourceBytes {
   public:
    SourceBytes(std::string&& str, std::size_t size) : buffer_(std::move(str)), line_starts_(size / 30 + 8) {
        build_line_starts();
    }

    static Result<SourceBytes> from_file(const char* path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return diagnostic_error(FileCode::file_not_found);
        std::string buf(std::istreambuf_iterator<char>(file), {});
        if (auto e = strip_bom(buf); !e) return diagnostic_error(e.error());
        auto size = buf.size();
        return size >= std::numeric_limits<std::uint32_t>::max()
                   ? diagnostic_error(FileCode::file_too_large)
                   : Result<SourceBytes>(std::in_place, std::move(buf), size);
    }

    char operator[](std::uint32_t index) const { return buffer_[index]; }

    char at(std::uint32_t index) const {
        assert(index <= buffer_.size());
        return buffer_[index];
    }
    std::string_view substr(size_t n_start, size_t n_len) const {
        return std::string_view(buffer_.substr(n_start, n_len));
    }

   private:
    static Result<void> strip_bom(std::string& buf) {
        unsigned char aa, ab, ac;
        if (auto len { buf.size() }; len >= 2) {
            aa = static_cast<unsigned char>(buf[0]), ab = static_cast<unsigned char>(buf[1]);
            if ((aa == 0xFF && ab == 0xFE) || (aa == 0xFE && ab == 0xFF))
                return diagnostic_error(FileCode::encoding_unsupported);
            if (len >= 3) {
                ac = static_cast<unsigned char>(buf[2]);
                if (aa == 0xEF && ab == 0xBB && ac == 0xBF) {
                    buf.erase(0, 3);
                } else if (len >= 4 && aa == 0x00 && ab == 0x00 && ac == 0xFE &&
                           static_cast<unsigned char>(buf[3]) == 0xFF)
                    return diagnostic_error(FileCode::encoding_unsupported);
            }
        }
        return {};
    }

    void build_line_starts() {
        for (std::size_t i {}; i < buffer_.size(); ++i) {
            char c { buffer_[i] };
            if (c == '\n') line_starts_.push_back(i + 1);
            else if (c == '\r') {
                if (buffer_[i + 1] == '\n') ++i;
                line_starts_.push_back(i + 1);
            }
        }
    }

    std::string buffer_;
    std::vector<std::uint32_t> line_starts_;
};

#endif