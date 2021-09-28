#include "uspeakframecontainer.h"

#include "helpers.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

constexpr std::size_t FrameHeaderSize = sizeof(std::uint16_t) + sizeof(std::uint16_t);

USpeakNative::USpeakFrameContainer::USpeakFrameContainer()
    : m_data()
{
}

bool USpeakNative::USpeakFrameContainer::fromData(std::span<const std::byte> data, std::uint16_t frameIndex)
{
    if (data.size() > UINT16_MAX || data.size() == 0) {
        return false;
    }

    m_data.resize(data.size() + FrameHeaderSize);

    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(m_data.data(), 0, frameIndex);
    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(m_data.data(), 2, (std::uint16_t)data.size());

    std::copy(data.begin(), data.end(), m_data.begin());

    return true;
}

bool USpeakNative::USpeakFrameContainer::decode(std::span<const std::byte> data, std::size_t offset)
{
    auto actualBegin = data.begin() + offset;
    std::size_t actualSize = data.size() - offset;

    if (actualSize < FrameHeaderSize) {
        fmt::print("Data size less then frame header size!\n");
        m_data.clear();
        return false;
    }

    std::uint16_t frameSize = USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(data.data(), 2) + FrameHeaderSize;
    if (frameSize > actualSize) {
        fmt::print("Header size invalid! (exceeds size of data)\n");
        m_data.clear();
        return false;
    }

    // Read frame
    m_data.resize(frameSize);
    std::copy(actualBegin, actualBegin + frameSize, m_data.begin());

    return true;
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::encodedData() const
{
    return m_data;
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::decodedData() const
{
    if (m_data.size() <= FrameHeaderSize) {
        return {};
    }

    return std::span<const std::byte>(m_data.begin() + FrameHeaderSize, m_data.end());
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameIndex() const
{
    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data.data(), 0);
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameSize() const
{
    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data.data(), 2);
}
