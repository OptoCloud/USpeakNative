#include "uspeakframecontainer.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

constexpr void ConvertToBytes(const std::uint8_t* data, std::size_t offset, std::uint16_t i) {
    *(std::uint16_t*)(data + offset) = i;
}
constexpr std::uint16_t ConvertToUint16(const std::uint8_t* data, std::size_t offset) {
    return *(std::uint16_t*)(data + offset);
}

constexpr std::size_t FrameHeaderSize = sizeof(std::uint16_t) + sizeof(std::uint16_t);

USpeakNative::USpeakFrameContainer::USpeakFrameContainer()
    : m_frameSize()
    , m_frameIndex()
    , m_frameData()
{
}

bool USpeakNative::USpeakFrameContainer::fromData(std::span<const uint8_t> data, std::uint16_t frameIndex)
{
    if (data.size() > UINT16_MAX || data.size() == 0) {
        return false;
    }

    m_frameSize = data.size();
    m_frameIndex = frameIndex;

    m_frameData.resize(data.size() + FrameHeaderSize);

    ConvertToBytes(data.data(), 0, m_frameIndex);
    ConvertToBytes(data.data(), 2, m_frameSize);

    memcpy(m_frameData.data() + FrameHeaderSize, data.data(), data.size());

    return true;
}

bool USpeakNative::USpeakFrameContainer::decode(std::span<const std::uint8_t> data, std::size_t offset)
{
    const std::uint8_t* actualData = data.data() + offset;
    std::size_t actualSize = data.size() - offset;

    if (actualSize < FrameHeaderSize) {
        fmt::print("Data size less then frame header size!\n");
        m_frameData.resize(0);
        return false;
    }

    // Read header
    m_frameIndex = ConvertToUint16(actualData, 0);
    m_frameSize =  ConvertToUint16(actualData, sizeof(std::uint16_t) );

    if (actualSize < m_frameSize + FrameHeaderSize) {
        fmt::print("Data size is less than frame header size!\n");
        m_frameData.resize(0);
        return false;
    }

    // Read frame
    m_frameData.resize(actualSize);
    memcpy(m_frameData.data(), actualData, actualSize);

    return true;
}

std::span<const std::uint8_t> USpeakNative::USpeakFrameContainer::encode()
{
    return m_frameData;
}

std::size_t USpeakNative::USpeakFrameContainer::encodedSize()
{
    return m_frameData.size();
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameSize()
{
    return m_frameSize;
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameIndex()
{
    return m_frameIndex;
}
