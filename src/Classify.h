#ifndef CLASSIFY_H
#define CLASSIFY_H

constexpr bool is_eol(char c) { return c == '\n' || c == '\r'; }

constexpr bool is_space(char c) { return c == '\t' || c == ' '; }
constexpr bool is_whitespace(char c) { return is_space(c) || is_eol(c); }

constexpr bool is_dec(char c) { return c >= '0' && c <= '9'; }
constexpr bool is_bin(char c) { return c == '0' || c == '1'; }
constexpr bool is_alhex(char c) { return (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
constexpr bool is_hex(char c) { return is_dec(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }

constexpr bool is_dec_post(char c) { return c == 'D' || c == 'd'; }
constexpr bool is_bin_post(char c) { return c == 'B' || c == 'b'; }
constexpr bool is_oct_post(char c) { return c == 'O' || c == 'o' || c == 'Q' || c == 'q'; }
constexpr bool is_hex_post(char c) { return c == 'H' || c == 'h'; }

constexpr bool is_float_post(char c) { return c == 'F' || c == 'f'; }
constexpr bool is_float_e(char c) { return c == 'E' || c == 'e'; }
constexpr bool is_sign(char c) { return c == '+' || c == '-'; }

constexpr bool is_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
constexpr bool is_alnum(char c) { return is_alpha(c) || is_dec(c); }
constexpr bool is_alpha_nonhex(char c) { return (c > 'F' && c <= 'Z') || (c > 'f' && c <= 'z'); }
constexpr bool is_ident_alnum(char c) { return is_alnum(c) || c == '_'; }
constexpr bool is_ident_nondigit(char c) { return is_alpha(c) || c == '_'; }
constexpr bool is_ident_nonhex(char c) { return is_alpha_nonhex(c) || c == '_'; }

constexpr bool is_non_kw(char c) { return is_dec(c) || c == '_'; }

constexpr bool is_a_or_c(char c) { return c == 'A' || c == 'a' || c == 'C' || c == 'c'; }
constexpr bool is_b_or_r(char c) { return c == 'b' || c == 'B' || c == 'r' || c == 'R'; }
constexpr bool is_f(char c) { return c == 'f' || c == 'F'; }
constexpr bool is_an_x(char c) { return c == 'x' || c == 'X'; }

constexpr bool is_lf(char c) { return c == '\n'; }
constexpr bool is_cr(char c) { return c == '\r'; }
constexpr bool is_at_sign(char c) { return c == '@'; }
constexpr bool is_dollar(char c) { return c == '$'; }
constexpr bool is_period(char c) { return c == '.'; }
constexpr bool is_underscore(char c) { return c == '_'; }

#endif