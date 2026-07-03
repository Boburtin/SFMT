#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <algorithm>
#include <array>
#include <cassert>
#include <string_view>

#include "Basics.h"

constexpr std::string_view kw_src[] = {
    "common", "forward", "reverse", "local",   "while", "repeat",   "match",     "macro",
    "struc",  "restruc", "struct",  "end",     "if",    "else",     "include",   "db",
    "dw",     "dd",      "dq",      "dt",      "rb",    "rw",       "rd",        "rq",
    "byte",   "word",    "dword",   "qword",   "tword", "yword",    "zword",     "ptr",
    "equ",    "section", "segment", "code",    "data",  "readable", "writeable", "executable",
    "entry",  "extern",  "public",  "display", "org",   "bits",     "defined",   "fix",
    "purge",  "restore", "format",  "console", "gui"
};

constexpr auto kw_first = std::to_underlying(Tag::kw_common);
constexpr auto kw_count = std::size(kw_src);
static_assert(static_cast<uint8_t>(Tag::kw_gui) - static_cast<uint8_t>(kw_first) + 1 == kw_count,
              "kw_src out of sync with Tag enum block");

constexpr uint8_t kw_max_len = []() consteval {
    uint8_t m {};
    for (auto s : kw_src) m = std::max<uint8_t>(m, s.size());
    return m;
}();

constexpr size_t W { (kw_max_len + 7) / 8 };  // hm words

struct KwWords {
    uint64_t w[W];
};

constexpr KwWords kw_pack(std::string_view s) {  // pack bytes into (2 * uint64_t) k
    KwWords k {};
    for (size_t i {}; i < s.size(); ++i) { k.w[i / 8] |= uint64_t(uint8_t(s[i])) << (8 * (i % 8)); }
    return k;
}

struct KwKey {
    KwWords words;
    Tag tag;
};
constexpr auto kw_keys = []() consteval {
    std::array<KwKey, kw_count> a {};
    for (size_t i {}; i < kw_count; ++i) {
        a[i] = { kw_pack(kw_src[i]), Tag(uint8_t(kw_first) + i) };
    }
    return a;
}();

constexpr size_t np2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}
constexpr size_t kw_slots { np2(8 * kw_count) };
constexpr size_t kw_mask { kw_slots - 1 };

constexpr uint64_t kw_hash(const KwWords& k, uint64_t seed) {
    uint64_t h { seed + 0x9E3779B97F4A7C15ULL };
    for (size_t i {}; i < W; ++i) {
        h ^= k.w[i];
        h *= 0xFF51AFD7ED558CCDULL;
        h ^= h >> 33;
    }
    return h;
}

constexpr uint64_t kw_seed = []() consteval {
    uint64_t seed {};
    while (true) {
        std::array<bool, kw_slots> used {};
        bool ok { true };
        for (const auto& k : kw_keys) {
            size_t s { kw_hash(k.words, seed) & kw_mask };
            if (used[s]) {
                ok = false;
                break;
            }
            used[s] = true;
        }
        if (ok) return seed;
        ++seed;
    }
}();

alignas(16) constexpr auto kw_word = []() consteval {
    std::array<KwWords, kw_slots> t {};
    for (const auto& k : kw_keys) t[kw_hash(k.words, kw_seed) & kw_mask] = k.words;
    return t;
}();

auto kw_tag = []() consteval {
    std::array<Tag, kw_slots> t {};
    for (auto& x : t) { x = Tag::identifier; }
    for (const auto& k : kw_keys) { t[kw_hash(k.words, kw_seed) & kw_mask] = k.tag; }
    return t;
}();

inline Tag kw_lookup(const KwWords& buf) {
    size_t s { kw_hash(buf, kw_seed) & kw_mask };
    const KwWords& e = kw_word[s];
    bool eq { true };
    for (size_t i {}; i < W; ++i) eq &= (e.w[i] == buf.w[i]);
    return eq ? kw_tag[s] : Tag::identifier;
}

#endif