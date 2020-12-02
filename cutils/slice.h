#pragma once

#include <ctype.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "chash.h"

namespace cutils {

class Slice {
public:
    // Create an empty slice.
    Slice() : data_(""), size_(0) { }

    // Create a slice that refers to d[0,n-1].
    Slice(const char* d, size_t n) : data_(d), size_(n) { }

    // Create a slice that refers to the contents of "s"
    Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }

    // Create a slice that refers to the contents of "s"
    Slice(const std::string* s) : data_(s->data()), size_(s->size()) { }

    // Create a slice that refers to s[0,strlen(s)-1]
    Slice(const char* s) : data_(s), size_(strlen(s)) { }

    // Return a pointer to the beginning of the referenced data
    const char* data() const { return data_; }

    void assign(const char* d, size_t n) {
        data_ = d;
        size_ = n;
    }

    // Return the length (in bytes) of the referenced data
    size_t size() const { return size_; }

    // Return true iff the length of the referenced data is zero
    bool empty() const { return size_ == 0; }

    // Return the ith byte in the referenced data.
    // REQUIRES: n < size()
    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    // Change this slice to refer to an empty array
    void clear() { data_ = ""; size_ = 0; }

    // Drop the first "n" bytes from this slice.
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    // Return a string that contains the copy of the referenced data.
    std::string ToString() const { return std::string(data_, size_); }
    std::string ToString(size_t pos, ssize_t len = -1) const { 
        if (len == -1) {
            assert(pos <= size_);
            return std::string(data_ + pos, size_ - pos);
        } else {
            assert(pos + len <= size_);
            return std::string(data_ + pos, len); 
        }
    }

    std::string ToHexString() const;

    // Three-way comparison.  Returns value:
    //   <  0 iff "*this" <  "b",
    //   == 0 iff "*this" == "b",
    //   >  0 iff "*this" >  "b"
    int compare(const Slice& b) const;

    // Return true iff "x" is a prefix of "*this"
    bool starts_with(const Slice& x) const {
        return ((size_ >= x.size_) &&
                (memcmp(data_, x.data_, x.size_) == 0));
    }

private:
    const char* data_;
    size_t size_;

    // Intentionally copyable
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
    return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
    const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
    int r = memcmp(data_, b.data_, min_len);
    if (r == 0) {
        if (size_ < b.size_) r = -1;
        else if (size_ > b.size_) r = +1;
    }
    return r;
}

template<class T>
inline void Split(const Slice& text, char c, 
                  T* res, bool skip_empty = false) {
    if (!res) return;
    res->clear();
    size_t beg = 0;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == c) {
            if (!skip_empty || i > beg) {
                res->insert(res->end(), {text.data() + beg, i - beg});
            }
            beg = i + 1;
        }
    }
}

inline std::string ToHexString(const Slice& s) {
    static const char* dic = "0123456789ABCDEF";
    std::string hex_str;
    auto size = s.size();
    auto data = s.data();
    hex_str.reserve(size * 2);
    for (size_t i = 0; i < size; ++i) {
        unsigned char c = data[i];
        hex_str.push_back(dic[c >> 4]);
        hex_str.push_back(dic[c & 15]);
    }
    return hex_str;
}

inline std::string Slice::ToHexString() const {
    return ::cutils::ToHexString(*this);
}

inline std::string ParseFromHexString(const Slice& s) {
    if (s.size() % 2 != 0) return "";
    std::string res;
    res.resize(s.size() / 2);
    auto convert_char = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 255;
    };
    for (size_t i = 0; i + 2 < s.size(); i += 2) {
        auto x = convert_char(s[i]);
        auto y = convert_char(s[i + 1]);
        if (x == 255 || y == 255) return "";
        res[i / 2] = (char)((x << 4) | y);
    }
    return res;
}

inline bool IsPrintable(const Slice& s) {
    bool ok = true;
    for (size_t i = 0; i < s.size() && ok; ++i) {
        ok = (isprint(s[i]) > 0);
    }
    return ok;
}

}  // namespace cutils



namespace std {

template<>
class hash<cutils::Slice> {
public:
    size_t operator()(const cutils::Slice& s) const {
        return cutils::CHash<size_t>(s.data(), s.size());
    }
};

} // namespace std;




