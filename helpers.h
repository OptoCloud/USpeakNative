#ifndef HELPERS_H
#define HELPERS_H

#include <concepts>
#include <cstdint>

namespace USpeakNative::Helpers {

template <typename T>
constexpr void ConvertToBytes(const void* data, std::size_t offset, T i) {
    static_assert(std::is_integral<T>());
    auto offsetData = (const std::uint8_t*)data + offset;
    auto typeAddress = (T*)offsetData;
    *typeAddress = i;
}

template <typename T>
constexpr T ConvertFromBytes(const void* data, std::size_t offset) {
    static_assert(std::is_integral<T>());
    auto offsetData = (const std::uint8_t*)data + offset;
    auto typeAddress = (T*)offsetData;
    return *typeAddress;
}

}

#endif // HELPERS_H
