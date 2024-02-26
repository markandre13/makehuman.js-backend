#pragma once

#include <format>
#include <iomanip>
#include <string_view>

namespace CORBA {

namespace detail {

template <typename Char>
struct basic_ostream_formatter : std::formatter<std::basic_string_view<Char>, Char> {
        template <typename T, typename OutputIt>
        auto format(const T &value, std::basic_format_context<OutputIt, Char> &ctx) const -> OutputIt {
            std::basic_stringstream<Char> ss;
            ss << value;
            return std::formatter<std::basic_string_view<Char>, Char>::format(ss.view(), ctx);
        }
};
using ostream_formatter = basic_ostream_formatter<char>;

}  // namespace detail

class blob_view : public std::basic_string_view<std::byte> {
    private:
        using base_t = std::basic_string_view<std::byte>;

    public:
        blob_view(const char *buffer) : base_t(reinterpret_cast<const std::byte *>(buffer), strlen(buffer)) {}
        blob_view(const void *buffer, size_t nbytes) : base_t(reinterpret_cast<const std::byte *>(buffer), nbytes) {}
        // friend ostream &operator<<(ostream &os, const blob_view &dt);
};

class blob : public std::basic_string<std::byte> {
    private:
        using base_t = std::basic_string<std::byte>;

    public:
        blob(const char *buffer) : base_t(reinterpret_cast<const std::byte *>(buffer), strlen(buffer)) {}
        blob(const void *buffer, size_t nbytes) : base_t(reinterpret_cast<const std::byte *>(buffer), nbytes) {}
        blob(const blob_view &value) : base_t(value) {}
        // friend ostream &operator<<(ostream &os, const blob_view &dt);
};

inline std::ostream &operator<<(std::ostream &os, const blob &dt) {
    os << std::hex << std::setfill('0');
    for (auto &b : dt) {
        os << std::setw(2) << (unsigned)b;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const blob_view &dt) {
    os << std::hex << std::setfill('0');
    for (auto &b : dt) {
        os << std::setw(2) << (unsigned)b;
    }
    return os;
}

}  // namespace CORBA

template <>
struct std::formatter<CORBA::blob_view> : CORBA::detail::ostream_formatter {};

template <>
struct std::formatter<CORBA::blob> : CORBA::detail::ostream_formatter {};
