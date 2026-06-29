#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <algorithm>
#include <array>
#include <string_view>

#include "Basics.h"

struct KwEntry {
    std::string_view text;
    Tag tag;
};

constexpr uint8_t kw_elements = 53;

/* constexpr auto kw_table = <...> makes this function consteval anyway but keep consteval for clarity */
consteval auto make_kw_table() {
    std::array<KwEntry, kw_elements> table { { { "common", Tag::kw_common },
                                               { "forward", Tag::kw_forward },
                                               { "reverse", Tag::kw_reverse },
                                               { "local", Tag::kw_local },
                                               { "while", Tag::kw_while },
                                               { "repeat", Tag::kw_repeat },
                                               { "match", Tag::kw_match },
                                               { "macro", Tag::kw_macro },
                                               { "struc", Tag::kw_struc },
                                               { "restruc", Tag::kw_restruc },
                                               { "struct", Tag::kw_struct },
                                               { "end", Tag::kw_end },
                                               { "if", Tag::kw_if },
                                               { "else", Tag::kw_else },
                                               { "include", Tag::kw_include },
                                               { "db", Tag::kw_db },
                                               { "dw", Tag::kw_dw },
                                               { "dd", Tag::kw_dd },
                                               { "dq", Tag::kw_dq },
                                               { "dt", Tag::kw_dt },
                                               { "rb", Tag::kw_rb },
                                               { "rw", Tag::kw_rw },
                                               { "rd", Tag::kw_rd },
                                               { "rq", Tag::kw_rq },
                                               { "byte", Tag::kw_byte },
                                               { "word", Tag::kw_word },
                                               { "dword", Tag::kw_dword },
                                               { "qword", Tag::kw_qword },
                                               { "tword", Tag::kw_tword },
                                               { "yword", Tag::kw_yword },
                                               { "zword", Tag::kw_zword },
                                               { "ptr", Tag::kw_ptr },
                                               { "equ", Tag::kw_equ },
                                               { "section", Tag::kw_section },
                                               { "segment", Tag::kw_segment },
                                               { "code", Tag::kw_code },
                                               { "data", Tag::kw_data },
                                               { "readable", Tag::kw_readable },
                                               { "writeable", Tag::kw_writeable },
                                               { "executable", Tag::kw_executable },
                                               { "entry", Tag::kw_entry },
                                               { "extern", Tag::kw_extern },
                                               { "public", Tag::kw_public },
                                               { "display", Tag::kw_display },
                                               { "org", Tag::kw_org },
                                               { "bits", Tag::kw_bits },
                                               { "defined", Tag::kw_defined },
                                               { "fix", Tag::kw_fix },
                                               { "purge", Tag::kw_purge },
                                               { "restore", Tag::kw_restore },
                                               { "format", Tag::kw_format },
                                               { "console", Tag::kw_console },
                                               { "gui", Tag::kw_gui } } };

    std::sort(table.begin(), table.end(), [](const KwEntry& a, const KwEntry& b) { return a.text < b.text; });

    return table;
}

constexpr auto kw_table = make_kw_table();

constexpr auto kw_cmp = [](const KwEntry& e, std::string_view key) { return e.text < key; };

#endif