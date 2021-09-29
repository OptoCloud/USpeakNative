#ifndef HELPERS_H
#define HELPERS_H

#include <concepts>
#include <cstdint>

namespace USpeakNative::Helpers {

constexpr void* AddOffset(void* ptr, std::size_t offset) {
    return (std::byte*)ptr + offset;
}
constexpr const void* AddOffset(const void* ptr, std::size_t offset) {
    return (std::byte*)ptr + offset;
}

template <typename T>
constexpr void ConvertToBytes(const void* data, std::size_t offset, T i) {
    static_assert(std::is_integral<T>());
    *(T*)AddOffset(data, offset) = i;
}

template <typename T>
constexpr T ConvertFromBytes(const void* data, std::size_t offset) {
    static_assert(std::is_integral<T>());
    return *(T*)AddOffset(data, offset);
}

}

#endif // HELPERS_H
