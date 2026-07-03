#ifndef BASICS_H
#define BASICS_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

using std::uint32_t, std::uint16_t, std::uint8_t, std::size_t;

/* primary token identifier | keep to tokens and keywords only */
enum class Tag : uint8_t {
    none = 0,
    eof,
    string,
    number,
    identifier,
    plus,
    minus,
    slash,
    asterisk,
    equal,
    l_angle_bracket,
    r_angle_bracket,
    l_angle_equal,
    r_angle_equal,
    l_shift,
    r_shift,
    l_paren,
    r_paren,
    l_bracket,
    r_bracket,
    l_brace,
    r_brace,
    colon,
    question,
    bang,
    comma,
    dot,
    pipe,
    ampersand,
    tilde,
    hash,
    backtick,
    backslash,
    dollar_sign,
    double_dollar_sign,
    at_at,
    at_back,
    at_forward,
    percent,
    endstmt,
    // keywords
    kw_common = 40,
    kw_forward,
    kw_reverse,
    kw_local,
    kw_while,
    kw_repeat,
    kw_match,
    kw_macro,
    kw_struc,
    kw_restruc,
    kw_struct,
    kw_end,
    kw_if,
    kw_else,
    kw_include,
    kw_db,
    kw_dw,
    kw_dd,
    kw_dq,
    kw_dt,
    kw_rb,
    kw_rw,
    kw_rd,
    kw_rq,
    kw_byte,
    kw_word,
    kw_dword,
    kw_qword,
    kw_tword,
    kw_yword,
    kw_zword,
    kw_ptr,
    kw_equ,
    kw_section,
    kw_segment,
    kw_code,
    kw_data,
    kw_readable,
    kw_writeable,
    kw_executable,
    kw_entry,
    kw_extern,
    kw_public,
    kw_display,
    kw_org,
    kw_bits,
    kw_defined,
    kw_fix,
    kw_purge,
    kw_restore,
    kw_format,
    kw_console,
    kw_gui
};

/* associated with Tag and SourceBytes handle via shared index in MultiArray */
struct TokenFlags {
    uint8_t bits;

    static constexpr uint8_t has_escape = 0b0000'0001;
    static constexpr uint8_t follows_space = 0b0000'0010;
    static constexpr uint8_t unterminated = 0b0000'0100;
    static constexpr uint8_t is_float = 0b0000'1000;
    static constexpr uint8_t has_overflow = 0b0001'0000;
    static constexpr uint8_t has_error = 0b0010'0000;
    static constexpr uint8_t radix_mask = 0b1100'0000;

    enum Radix : uint8_t { oct = 0b0100'0000, hex = 0b1000'0000, bin = 0b1100'0000 };

    bool has(uint8_t bit) const { return bits & bit; }
    void set(uint8_t bit) { bits |= bit; }
    void clear(uint8_t bit) { bits &= ~bit; }

    uint8_t operator|(uint8_t bit) const { return bits | bit; }
    uint8_t operator|(Radix r) const { return (bits & ~radix_mask) | r; }

    Radix radix() const { return static_cast<Radix>(bits & radix_mask); }
    void set_radix(Radix r) { bits = (bits & ~radix_mask) | r; }
};

/* contiguous allocated list */

template <class T>
class SingleArray {
   public:
    SingleArray(uint32_t n) : data_ { std::make_unique_for_overwrite<T[]>(n) }, size_ {}, cap_ { n } {}

    T& operator[](uint32_t index) { return data_[index]; }
    const T& operator[](uint32_t index) const { return data_[index]; }

    template <class... Args>
    void emplace(Args&&... args) {
        new (data_.get() + size_) T(std::forward<Args>(args)...);
        ++size_;
    }

    void push(T t) {
        new (data_.get() + size_) T(std::move(t));
        ++size_;
    }

    void shrink_fit() {
        cap_ = size_;
        auto buf = std::make_unique_for_overwrite<T[]>(size_);
        std::memcpy(buf.get(), data_.get(), size_ * sizeof(T));
        data_ = std::move(buf);
    }

   private:
    std::unique_ptr<T[]> data_;
    uint32_t size_;
    uint32_t cap_;
};

struct MultiArray {
    SingleArray<uint32_t> starts;
    SingleArray<Tag> tags;
    SingleArray<TokenFlags> flags;
};

#endif