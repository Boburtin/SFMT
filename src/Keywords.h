#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

/* Find out if idiomatic to include cstdint even when Basics has the alias defs */

#include "Basics.h"

/* NOTE: trying length bucket approach now but will want to switch to perfect hash functions */

constexpr std::string_view kw_src[] = {
    "common", "forward", "reverse", "local",   "while", "repeat",   "match",     "macro",
    "struc",  "restruc", "struct",  "end",     "if",    "else",     "include",   "db",
    "dw",     "dd",      "dq",      "dt",      "rb",    "rw",       "rd",        "rq",
    "byte",   "word",    "dword",   "qword",   "tword", "yword",    "zword",     "ptr",
    "equ",    "section", "segment", "code",    "data",  "readable", "writeable", "executable",
    "entry",  "extern",  "public",  "display", "org",   "bits",     "defined",   "fix",
    "purge",  "restore", "format",  "console", "gui"
};

constexpr auto kw_count = std::size(kw_src);
constexpr auto kw_first = std::to_underlying(Tag::kw_common);
static_assert(size_t(std::to_underlying(Tag::kw_gui) - kw_first + 1) == kw_count,
              "kw_src out of sync with Tag enum block");

constexpr uint8_t kw_max_len = []() consteval {
    uint8_t m {};
    for (auto s : kw_src) m = std::max<uint8_t>(m, s.size());
    return m;
}();
constexpr uint8_t kw_min_len = []() consteval {
    uint8_t m { 255 };
    for (auto s : kw_src) m = std::min<uint8_t>(m, s.size());
    return m;
}();
constexpr size_t W { (kw_max_len + 7) / 8 };  // 64-bit words per key

struct KwWords {
    uint64_t w[W];
};

constexpr KwWords kw_pack(std::string_view s) {  // pack bytes into (2 * uint64_t) k
    KwWords k {};
    for (size_t i {}; i < s.size(); ++i) { k.w[i / 8] |= uint64_t(uint8_t(s[i])) << (8 * (i % 8)); }
    return k;
}
// bad alignment. Fix.
struct KwKey {
    KwWords words;
    Tag tag;
    uint8_t len;
};

struct KwTables {
    std::array<KwWords, kw_count> words;
    std::array<Tag, kw_count> tag;
    std::array<uint8_t, kw_max_len + 2> len_off;
};

alignas(16) constexpr KwTables kw = []() consteval {
    std::array<KwKey, kw_count> a {};
    for (size_t i {}; i < kw_count; ++i) {
        a[i] = { kw_pack(kw_src[i]), Tag(kw_first + i), uint8_t(kw_src[i].size()) };
    }
    std::sort(a.begin(), a.end(), [](const KwKey& x, const KwKey& y) { return x.len < y.len; });

    KwTables t {};
    for (size_t i {}; i < kw_count; ++i) {
        t.words[i] = a[i].words;
        t.tag[i] = a[i].tag;
    }
    for (const auto& k : a) t.len_off[k.len + 1]++;
    for (size_t L { 1 }; L < t.len_off.size(); ++L) t.len_off[L] += t.len_off[L - 1];
    return t;
}();

inline Tag kw_lookup(const KwWords& buf, uint8_t len) {
    if (len < kw_min_len || len > kw_max_len) return Tag::identifier;
    for (uint8_t i = kw.len_off[len]; i < kw.len_off[len + 1]; ++i) {
        bool eq = true;
        for (size_t j {}; j < W; ++j) { eq &= (kw.words[i].w[j] == buf.w[j]); }
        if (eq) return kw.tag[i];
    }
    return Tag::identifier;
}

#endif