#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <algorithm>
#include <cstdint>
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
            seen_semicolon,
            seen_at,
            seen_cr,
            seen_angle_left,
            seen_angle_right,
            in_num_malformed,
            in_zero_start,
            in_unprefixed_hex,
            in_prefixed_hex,
            in_float,
            in_e_float,
            in_e_sign_float,
            in_float_e_digits,
            in_string_single,
            in_string_double,
            in_identifier,
            in_potential_kw,
        };

        State state {};
        TokenFlags flags {};
        uint32_t start { pos_ };

        char buf[16] {};
        uint8_t buf_len {};

        while (true) {
            char c { get() };
            switch (state) {
                case State::start:
                    switch (c) {
                        case '\t':
                        case ' ' : {
                            start = ++pos_;
                            flags.set(TokenFlags::follows_space);
                            continue;
                        }
                        case ':' : ++pos_; return { start, Tag::colon, flags };
                        case ',' : ++pos_; return { start, Tag::comma, flags };
                        case '-' : ++pos_; return { start, Tag::minus, flags };
                        case '+' : ++pos_; return { start, Tag::plus, flags };
                        case '/' : ++pos_; return { start, Tag::slash, flags };
                        case '*' : ++pos_; return { start, Tag::asterisk, flags };
                        case '`' : ++pos_; return { start, Tag::backtick, flags };
                        case '?' : ++pos_; return { start, Tag::question, flags };
                        case '#' : ++pos_; return { start, Tag::hash, flags };
                        case '!' : ++pos_; return { start, Tag::bang, flags };
                        case '|' : ++pos_; return { start, Tag::pipe, flags };
                        case '.' : ++pos_; return { start, Tag::dot, flags };
                        case '&' : ++pos_; return { start, Tag::ampersand, flags };
                        case '[' : ++pos_; return { start, Tag::l_bracket, flags };
                        case ']' : ++pos_; return { start, Tag::r_bracket, flags };
                        case '(' : ++pos_; return { start, Tag::l_paren, flags };
                        case ')' : ++pos_; return { start, Tag::r_paren, flags };
                        case '{' : ++pos_; return { start, Tag::l_brace, flags };
                        case '}' : ++pos_; return { start, Tag::r_brace, flags };
                        case '\0': return { start, Tag::eof, flags };
                        case '\n': return { pos_++, Tag::endstmt, flags };
                        case '\r': ++pos_, state = State::seen_cr; continue;
                        case '@' : ++pos_, state = State::seen_at; continue;
                        case '<' : ++pos_, state = State::seen_angle_left; continue;
                        case '>' : ++pos_, state = State::seen_angle_right; continue;
                        case '\'': ++pos_, state = State::in_string_single; continue;
                        case '"' : ++pos_, state = State::in_string_double; continue;
                        case '$' : ++pos_, state = State::seen_dollar_sign; continue;
                        case '0' : ++pos_, state = State::seen_zero_start; continue;
                        case '_' : ++pos_, state = State::in_identifier; continue;
                        case ';' : ++pos_, state = State::seen_semicolon; continue;
                        default  : {
                            if (is_alpha(c)) {
                                ++pos_;
                                state = State::in_potential_kw;
                                buf[buf_len++] = c | 0x20;
                                continue;
                            }
                        }
                    }

                case State::seen_dollar_sign:
                    ++pos_;
                    if (!is_hex(c))
                        return { start, is_dollar(c) ? Tag::double_dollar_sign : Tag::dollar_sign, flags };
                    state = State::in_prefixed_hex;
                    continue;

                case State::seen_zero_start:
                    ++pos_;
                    if (is_x(c)) state = State::in_prefixed_hex;
                    else if (is_period(c)) state = State::in_float;
                    else if (is_hex_post(c)) state = State::seen_hex_post;
                    else if (is_dec_post(c)) state = State::seen_dec_post;
                    else if (is_oct_post(c)) state = State::seen_oct_post;
                    else if (is_bin_post(c)) state = State::seen_bin_post;
                    else if (is_float_post(c)) state = State::seen_float_post;
                    else if (is_float_e(c)) state = State::in_e_float;
                    else if (to_lower_comp(c, 'A') || to_lower_comp(c, 'C')) state = State::in_unprefixed_hex;
                    else if (is_dec(c)) state = State::in_zero_start;
                    else return { start, Tag::number, flags };

                case State::seen_hex_post:
                    ++pos_;
                    if (!is_ident_alnum(c)) {
                        flags.set_radix(TokenFlags::hex);
                        return { start, Tag::number, flags };
                    }
                    state = State::in_num_malformed;
                    continue;

                case State::seen_oct_post:
                    ++pos_;
                    if (!is_ident_alnum(c)) {
                        flags.set_radix(TokenFlags::hex);
                        return { start, Tag::number, flags };
                    }
                    state = State::in_num_malformed;
                    continue;

                case State::seen_bin_post:
                    ++pos_;
                    if (is_hex(c)) {
                        state = State::in_unprefixed_hex;
                        continue;
                    }
                    if (is_non_hex_alpha(c) || c == '_') {
                        state = State::in_num_malformed;
                        continue;
                    }
                    flags.set_radix(TokenFlags::bin);
                    return { start, Tag::number, flags };

                case State::seen_dec_post:
                    ++pos_;
                    if (is_hex(c)) {
                        state = State::in_unprefixed_hex;
                        continue;
                    }
                    if (is_non_hex_alpha(c) || c == '_') {
                        state = State::in_num_malformed;
                        continue;
                    }
                    return { start, Tag::number, flags };

                case State::seen_semicolon:
                    if (c == '\r') {
                        state = State::seen_cr;
                        start = pos_++;
                        continue;
                    }
                    if (c == '\n') { return { pos_++, Tag::endstmt, flags }; }
                    ++pos_;
                    continue;

                case State::seen_at:
                    ++pos_;
                    if (c == '@') { return { start, Tag::at_at, flags }; }
                    if (is_fw(c)) { return { start, Tag::at_fw, flags }; }
                    if (is_bk(c)) { return { start, Tag::at_bk, flags }; }
                    return { start, Tag::none, flags };

                case State::seen_cr:
                    if (c == '\n') ++pos_;
                    return { start, Tag::endstmt, flags };

                case State::seen_angle_left:
                case State::seen_angle_right:
                case State::in_num_malformed:
                case State::in_zero_start:
                case State::in_unprefixed_hex:
                case State::in_prefixed_hex:
                case State::in_float:
                case State::in_e_float:
                case State::in_e_sign_float:
                case State::in_float_e_digits:
                case State::in_string_single:
                case State::in_string_double:
                case State::in_identifier:
                case State::in_potential_kw:
            }
        }
    }

    const SourceBytes& src_;
    uint32_t pos_ = 0;
    /* 4 bytes reserved: spend on anything useful*/
};

#endif