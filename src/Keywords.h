#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <algorithm>
#include <array>
#include <cassert>
#include <string_view>

#include "Basics.h"

constexpr std::string_view kw_src[] = {
    "common",  "forward",  "reverse",   "local",      "while",   "repeat",  "match",   "macro",   "struc",
    "restruc", "struct",   "end",       "if",         "else",    "include", "db",      "dw",      "dd",
    "dq",      "dt",       "rb",        "rw",         "rd",      "rq",      "byte",    "word",    "dword",
    "qword",   "tword",    "yword",     "zword",      "ptr",     "equ",     "section", "segment", "code",
    "data",    "readable", "writeable", "executable", "entry",   "extern",  "public",  "display", "org",
    "bits",    "defined",  "fix",       "purge",      "restore", "format",  "console", "gui"
};

constexpr uint8_t kw_max_len = []() consteval {
    uint8_t m {};
    for (auto sv : kw_src) m = std::max<uint8_t>(m, sv.size());
    return m;
}();

struct KeywordEntry {
    char text[kw_max_len];
    uint8_t len;
    Tag tag;
};

constexpr auto kw_first = std::to_underlying(Tag::kw_common);

constexpr auto kw_table = []() consteval {
    std::array<KeywordEntry, std::size(kw_src)> t {};
    for (uint8_t i {}; i < std::size(kw_src); ++i) {
        auto sv = kw_src[i];
        std::copy(sv.begin(), sv.end(), t[i].text);
        t[i].len = static_cast<uint8_t>(sv.size());
        t[i].tag = static_cast<Tag>(kw_first + i);
    }
    std::sort(t.begin(), t.end(), [](const KeywordEntry& a, const KeywordEntry& b) {
        return std::string_view(a.text, a.len) < std::string_view(b.text, b.len);
    });
    return t;
}();

#endif