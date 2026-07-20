#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string_view>

#include "Classify.h"
#include "Keywords.h"
#include "SourceBytes.h"
// test ->
class Tokenizer {
   public:
    Tokenizer(const SourceBytes& src) : src_(src) {}

    void scan(MultiArray& out) {
        while (true) {
            auto [start, tag, flags] = next();
            out.starts.push(start);
            out.tags.push(tag);
            out.flags.push(flags);
            if (tag == Tag::eof) break;
        }
    }

    char get() const { return src_[pos_]; }

   private:
    struct Token {
        uint32_t start;
        Tag tag;
        TokenFlags flags;
    };

    Token next() {
        enum class State : uint16_t {
            start,
            seen_dollar_sign,
            seen_zero_start,
            seen_hex_post,
            seen_oct_post,
            seen_bin_post,
            seen_dec_post,
            seen_float_post,
            seen_float_post_after_dot,
            seen_semicolon,
            seen_at,
            seen_cr,
            seen_angle_left,
            seen_angle_right,
            seen_backslash,
            seen_cr_after_bslash,
            in_space_after_bslash,
            in_num_malformed,
            in_zero_start,
            in_nonzero_start,
            in_unprefixed_hex,
            in_prefixed_hex,
            in_float,
            in_float_e,
            seen_float_e_sign,
            in_float_e_digits,
            in_string_single,
            in_string_double,
            in_identifier,
            in_potential_kw,
            temp_done
        };

        uint32_t start { pos_ };
        TokenFlags flags {};

        State state {};

        char buf[8 * W] {};
        uint8_t buf_len {};

        while (true) {  // continue jumps to here
            char c { get() };
            switch (state) {
                case State::start: {
                    switch (c) {
                        case '\t':
                        case ' ' : {
                            start = ++pos_;
                            flags.set(TokenFlags::follows_space);
                            continue;
                        }
                        case '-' : return { (++pos_, start), Tag::minus, flags };
                        case '+' : return { (++pos_, start), Tag::plus, flags };
                        case '/' : return { (++pos_, start), Tag::slash, flags };
                        case '*' : return { (++pos_, start), Tag::asterisk, flags };
                        case '%' : return { (++pos_, start), Tag::percent, flags };
                        case '=' : return { (++pos_, start), Tag::equal, flags };
                        case '`' : return { (++pos_, start), Tag::backtick, flags };
                        case '?' : return { (++pos_, start), Tag::question, flags };
                        case '#' : return { (++pos_, start), Tag::hash, flags };
                        case '!' : return { (++pos_, start), Tag::bang, flags };
                        case '|' : return { (++pos_, start), Tag::pipe, flags };
                        case '.' : return { (++pos_, start), Tag::dot, flags };
                        case ':' : return { (++pos_, start), Tag::colon, flags };
                        case ',' : return { (++pos_, start), Tag::comma, flags };
                        case '~' : return { (++pos_, start), Tag::tilde, flags };
                        case '&' : return { (++pos_, start), Tag::ampersand, flags };
                        case '[' : return { (++pos_, start), Tag::l_bracket, flags };
                        case ']' : return { (++pos_, start), Tag::r_bracket, flags };
                        case '(' : return { (++pos_, start), Tag::l_paren, flags };
                        case ')' : return { (++pos_, start), Tag::r_paren, flags };
                        case '{' : return { (++pos_, start), Tag::l_brace, flags };
                        case '}' : return { (++pos_, start), Tag::r_brace, flags };
                        case '\n': return { (++pos_, start), Tag::endstmt, flags };
                        case '\0': {
                            if (flags.has(TokenFlags::has_escape)) flags.set(TokenFlags::has_error);
                            return { pos_, Tag::eof, flags };
                        }
                        case '\r': state = (++pos_, State::seen_cr); continue;
                        case '@' : state = (++pos_, State::seen_at); continue;
                        case '_' : state = (++pos_, State::in_identifier); continue;
                        case '\\': state = (++pos_, State::seen_backslash); continue;
                        case ';' : state = (++pos_, State::seen_semicolon); continue;
                        case '0' : state = (++pos_, State::seen_zero_start); continue;
                        case '<' : state = (++pos_, State::seen_angle_left); continue;
                        case '>' : state = (++pos_, State::seen_angle_right); continue;
                        case '"' : state = (++pos_, State::in_string_double); continue;
                        case '$' : state = (++pos_, State::seen_dollar_sign); continue;
                        case '\'': state = (++pos_, State::in_string_single); continue;
                        default  : {
                            if (is_alpha(c)) {
                                buf[buf_len++] = c | 0x20;
                                ++pos_;
                                state = State::in_potential_kw;
                                continue;
                            }
                            if (is_dec(c)) {
                                ++pos_;
                                state = State::in_nonzero_start;
                                continue;
                            }
                        }
                    }
                }

                case State::seen_dollar_sign: {
                    if (is_hex(c)) {
                        state = (++pos_, State::in_prefixed_hex);
                        continue;
                    }
                    return is_dollar(c) ? Token { (++pos_, start), Tag::double_dollar_sign, flags }
                                        : Token { start, Tag::dollar_sign, flags };
                }

                case State::seen_zero_start: {
                    state = is_an_x(c)             ? State::in_prefixed_hex
                            : is_period(c)         ? State::in_float
                            : is_hex_post(c)       ? State::seen_hex_post
                            : is_dec_post(c)       ? State::seen_dec_post
                            : is_oct_post(c)       ? State::seen_oct_post
                            : is_bin_post(c)       ? State::seen_bin_post
                            : is_float_post(c)     ? State::seen_float_post
                            : is_float_e(c)        ? State::in_float_e
                            : is_a_or_c(c)         ? State::in_zero_start
                            : is_ident_nondigit(c) ? State::in_num_malformed
                                                   : State::start;
                    if (state != State::start) {
                        ++pos_;
                        continue;
                    }
                    return { start, Tag::number, flags };
                }

                case State::seen_hex_post: {
                    if (!is_ident_alnum(c)) {
                        return { start, Tag::number, flags | TokenFlags::hex };
                    }
                    ++pos_;
                    state = State::in_num_malformed;
                    continue;
                }

                case State::seen_oct_post: {
                    if (!is_ident_alnum(c)) {
                        return { start, Tag::number, flags | TokenFlags::oct };
                    }
                    ++pos_;
                    state = State::in_num_malformed;
                    continue;
                }

                case State::seen_bin_post: {
                    if (!is_ident_alnum(c)) {
                        return { start, Tag::number, flags | TokenFlags::bin };
                    }
                    ++pos_;
                    state = is_hex(c) ? State::in_unprefixed_hex : State::in_num_malformed;
                    continue;
                }

                case State::seen_dec_post: {
                    if (!is_ident_alnum(c)) { return { start, Tag::number, flags }; }
                    ++pos_;
                    state = is_hex(c) ? State::in_unprefixed_hex : State::in_num_malformed;
                    continue;
                }

                case State::seen_float_post: {
                    if (!is_ident_alnum(c)) {
                        flags.set(TokenFlags::is_float);
                        return { start, Tag::number, flags };
                    }
                    ++pos_;
                    state = is_hex(c) ? State::in_unprefixed_hex : State::in_num_malformed;
                    continue;
                }

                case State::seen_float_post_after_dot: {
                }

                case State::seen_semicolon: {
                    if (c == '\n') { return { pos_++, Tag::endstmt, flags }; }
                    if (c == '\0') { return { pos_, Tag::eof, flags }; }
                    if (c == '\r') {
                        start = pos_;
                        state = State::seen_cr;
                    }
                    ++pos_;
                    continue;
                }

                case State::seen_at: {
                    return is_at_sign(c)  ? Token { (++pos_, start), Tag::at_at, flags }
                           : is_f(c)      ? Token { (++pos_, start), Tag::at_forward, flags }
                           : is_b_or_r(c) ? Token { (++pos_, start), Tag::at_back, flags }
                                          : Token { start, Tag::none, flags };
                }

                case State::seen_cr: {
                    if (c == '\n') ++pos_;
                    return { start, Tag::endstmt, flags };
                }

                case State::seen_angle_left: {
                    return c == '<' ? Token { (++pos_, start), Tag::l_shift, flags }
                                    : Token { start, Tag::l_angle_bracket, flags };
                }

                case State::seen_angle_right: {
                    return c == '>' ? Token { (++pos_, start), Tag::r_shift, flags }
                                    : Token { start, Tag::r_angle_bracket, flags };
                }

                case State::seen_backslash: {
                    if (is_space(c)) {
                        ++pos_;
                        state = State::in_space_after_bslash;
                        continue;
                    }
                    if (is_cr(c)) {
                        ++pos_;
                        state = State::seen_cr_after_bslash;
                        continue;
                    }
                    if (is_lf(c)) {
                        ++pos_;
                        start = pos_;
                        state = State::start;
                        continue;
                    }
                    flags.set(TokenFlags::has_escape);
                    state = State::start;
                    continue;
                }

                case State::seen_cr_after_bslash: {
                    if (is_lf(c)) { ++pos_; }
                    state = State::start;
                    start = pos_;
                    continue;
                }

                case State::in_space_after_bslash: {
                    if (is_space(c)) {
                        ++pos_;
                        continue;
                    }
                    if (is_cr(c)) {
                        ++pos_;
                        state = State::seen_cr_after_bslash;
                        continue;
                    }
                    if (is_lf(c)) {
                        ++pos_;
                        start = pos_;
                        state = State::start;
                        continue;
                    }
                    flags.set(TokenFlags::has_error);
                    start = pos_;
                    state = State::start;
                    continue;
                }

                case State::in_num_malformed: {
                    if (is_ident_alnum(c)) {
                        ++pos_;
                        continue;
                    }
                    return { start, Tag::number, flags | TokenFlags::has_error };
                }

                case State::in_zero_start: {
                    state = is_period(c)           ? State::in_float
                            : is_hex_post(c)       ? State::seen_hex_post
                            : is_dec_post(c)       ? State::seen_dec_post
                            : is_oct_post(c)       ? State::seen_oct_post
                            : is_bin_post(c)       ? State::seen_bin_post
                            : is_float_post(c)     ? State::seen_float_post
                            : is_float_e(c)        ? State::in_float_e
                            : is_a_or_c(c)         ? State::in_unprefixed_hex
                            : is_ident_nondigit(c) ? State::in_num_malformed
                                                   : State::in_zero_start;
                    if (state != State::in_zero_start || is_dec(c)) {
                        ++pos_;
                        continue;
                    }
                    return { start, Tag::number, flags };
                }

                case State::in_unprefixed_hex: {
                    state = is_hex_post(c)       ? State::seen_hex_post
                            : is_ident_nonhex(c) ? State::in_num_malformed
                                                 : State::in_unprefixed_hex;
                    if (state != State::in_unprefixed_hex || is_hex(c)) {
                        ++pos_;
                        continue;
                    }
                    return { start, Tag::number, flags | TokenFlags::has_error };
                }

                case State::in_prefixed_hex: {
                    if (is_ident_alnum(c)) {
                        if (is_ident_nonhex(c)) { state = State::in_num_malformed; }
                        ++pos_;
                        continue;
                    }
                    flags.set_radix(TokenFlags::hex);
                    return { start, Tag::number, flags };
                }

                case State::in_float: {
                    state = is_float_post(c)       ? State::seen_float_post_after_dot
                            : is_float_e(c)        ? State::in_float_e
                            : is_ident_nondigit(c) ? State::in_num_malformed
                                                   : State::in_float;
                    if (state != State::in_float || is_dec(c)) {
                        ++pos_;
                        continue;
                    }
                    flags.set(TokenFlags::is_float);
                    return { start, Tag::number, flags };
                }

                case State::in_float_e: {
                    state = is_dec(c)    ? State::in_float_e_digits
                            : is_sign(c) ? State::seen_float_e_sign
                                         : State::in_num_malformed;
                    if (state == State::in_num_malformed) {
                        flags.set(TokenFlags::is_float);
                        if (!is_ident_alnum(c)) {
                            flags.set(TokenFlags::has_error);
                            return { start, Tag::number, flags };
                        }
                    }
                    ++pos_;
                    continue;
                }

                case State::seen_float_e_sign: {
                    if (is_dec(c)) {
                        ++pos_;
                        state = State::in_float_e_digits;
                        continue;
                    }
                    flags.set(TokenFlags::has_error);
                    flags.set(TokenFlags::is_float);
                    return { start, Tag::number, flags };
                }

                case State::in_float_e_digits: {
                    if (is_ident_alnum(c)) {
                        if (!is_dec(c)) { state = State::in_num_malformed; }
                        ++pos_;
                        continue;
                    }
                    flags.set(TokenFlags::is_float);
                    return { start, Tag::number, flags };
                }

                case State::in_string_single: {
                    if (c == '\0') {
                        flags.set(TokenFlags::unterminated);
                    } else {
                        ++pos_;
                        if (c != '\'') { continue; }
                    }
                    return { start, Tag::string, flags };
                }

                case State::in_string_double: {
                    if (c == '\0') {
                        flags.set(TokenFlags::unterminated);
                    } else {
                        ++pos_;
                        if (c == '"') { continue; }
                    }
                    return { start, Tag::string, flags };
                }

                case State::in_identifier: {
                    if (is_ident_alnum(c)) {
                        ++pos_;
                        continue;
                    }
                    return { start, Tag::identifier, flags };
                }

                case State::in_potential_kw: {
                    if (is_alpha(c)) {
                        if (buf_len == kw_max_len) {
                            state = State::in_identifier;
                            continue;
                        }
                        buf[buf_len++] = c | 0x20;
                        ++pos_;
                        continue;
                    }
                    if (is_non_kw(c)) {
                        state = State::in_identifier;
                        continue;
                    }
                    KwWords bw {};
                    std::memcpy(bw.w, buf, sizeof(bw.w));
                    return { start, kw_lookup(bw, buf_len), flags };
                }
                default: continue;
            }
        }
    }

    const SourceBytes& src_;
    uint32_t pos_ = 0;
    /* 4 bytes reserved to spend on more flags */
};

#endif