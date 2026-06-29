#ifndef CLASSIFY_H
#define CLASSIFY_H

constexpr bool is_eof(char c) { return c == '\0'; }
constexpr bool is_eol(char c) { return c == '\n' || c == '\r'; }

constexpr bool is_space(char c) { return c == '\t' || c == ' '; }
constexpr bool is_whitespace(char c) { return is_space(c) || is_eol(c); }

constexpr bool is_separator(char c) { return c == '\\' || c == ';' || is_eof(c) || is_whitespace(c); }

constexpr bool is_dec(char c) { return c >= '0' && c <= '9'; }
constexpr bool is_bin(char c) { return c == '0' || c == '1'; }
constexpr bool is_oct(char c) { return c >= '0' && c <= '7'; }
constexpr bool is_alhex(char c) { return (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
constexpr bool is_hex(char c) { return is_dec(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }

constexpr bool is_non_hex_alpha(char c) { return (c >= 'F' && c <= 'Z') || (c >= 'f' && c <= 'z'); }

constexpr bool is_dec_post(char c) { return c == 'D' || c == 'd'; }
constexpr bool is_bin_post(char c) { return c == 'B' || c == 'b'; }
constexpr bool is_oct_post(char c) { return c == 'O' || c == 'o' || c == 'Q' || c == 'q'; }
constexpr bool is_hex_post(char c) { return c == 'H' || c == 'h'; }

constexpr bool is_float_point(char c) { return c == '.' || c == 'E' || c == 'e'; }
constexpr bool is_float_post(char c) { return c == 'F' || c == 'f'; }
constexpr bool is_float_e(char c) { return c == 'E' || c == 'e'; }

constexpr bool is_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
constexpr bool is_alnum(char c) { return is_alpha(c) || is_dec(c); }
constexpr bool is_ident_alnum(char c) { return is_alnum(c) || c == '_'; }
constexpr bool is_non_kw(char c) { return is_dec(c) || c == '_'; }

constexpr bool is_postfix(char c) {
    return (c == 'B' || c == 'b') || (c == 'D' || c == 'd') || (c == 'H' || c == 'h') ||
           (c == 'O' || c == 'o') || (c == 'Q' || c == 'q');
}

constexpr bool is_bk(char c) { return c == 'b' || c == 'B' || c == 'r' || c == 'R'; }
constexpr bool is_fw(char c) { return c == 'f' || c == 'F'; }
constexpr bool is_x(char c) { return c == 'x' || c == 'X'; }
constexpr bool is_lf(char c) { return c == '\n'; }
constexpr bool is_at(char c) { return c == '@'; }
constexpr bool is_dollar(char c) { return c == '$'; }

constexpr bool to_lower_comp(char a, char upperchar) { return a == upperchar || a == (upperchar | 0x20); }

constexpr bool lower_char(char c) { return c | 0x20; }

constexpr bool is_period(char c) { return c == '.'; }

#endif