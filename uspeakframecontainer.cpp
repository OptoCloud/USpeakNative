#include "uspeakframecontainer.h"

#include "helpers.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <limits>

constexpr std::size_t USPEAKFRAME_HEADERSIZE = sizeof(std::uint16_t) + sizeof(std::uint16_t);

constexpr bool IsInvalidOpusDataSize(std::size_t size) {
    return size > UINT16_MAX || size <= 0;
}
inline std::size_t GetUSpeakFrameSize(std::span<const std::byte> frameData) {
    if (frameData.size() < USPEAKFRAME_HEADERSIZE) {
        fmt::print("[USpeakNative] Data size less then frame header size!\n");
        return 0;
    }

    std::size_t opusDataSize = static_cast<std::size_t>(USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(frameData.data(), 2));
    if (IsInvalidOpusDataSize(opusDataSize)) {
        fmt::print("[USpeakNative] Opus data size invalid!\n");
        return 0;
    }

    std::size_t frameSize = opusDataSize + USPEAKFRAME_HEADERSIZE;
    if (frameSize > frameData.size()) {
        fmt::print("[USpeakNative] Header size invalid! (exceeds size of data)\n");
        return 0;
    }

    return frameSize;
}
inline std::size_t WriteContainerImpl(std::vector<std::byte>& frameData, std::size_t frameDataOffset, std::span<const std::byte> opusData, std::uint16_t frameIndex) {
    if (IsInvalidOpusDataSize(opusData.size())) {
        fmt::print("[USpeakNative] Opus data size invalid!\n");
        return 0;
    }

    std::size_t frameSize = opusData.size() + USPEAKFRAME_HEADERSIZE;

    frameData.resize(frameDataOffset + frameSize);

    auto frameSpan = std::span(frameData).subspan(frameDataOffset, frameSize);

    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(frameSpan.data(), 0, frameIndex);
    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(frameSpan.data(), 2, static_cast<std::uint16_t>(opusData.size()));
    std::copy(opusData.begin(), opusData.end(), frameSpan.begin() + USPEAKFRAME_HEADERSIZE);

    return frameSize;
}
inline std::size_t ReadContainerImpl(std::vector<std::byte>& opusData, std::uint16_t& frameIndex, std::span<const std::byte> frameData) {
    std::size_t frameSize = GetUSpeakFrameSize(frameData);
    if (frameSize == 0) return 0;

    std::size_t opusDataSize = frameSize - USPEAKFRAME_HEADERSIZE;

    auto opusDataView = frameData.subspan(USPEAKFRAME_HEADERSIZE, opusDataSize);

    // Read frame
    opusData.assign(opusDataView.begin(), opusDataView.end());

    return frameSize;
}

std::size_t USpeakNative::USpeakFrameContainer::WriteContainer(std::vector<std::byte>& frameData, std::size_t frameDataOffset, std::span<const std::byte> opusData, std::uint16_t frameIndex)
{
    return WriteContainerImpl(frameData, frameDataOffset, opusData, frameIndex);
}

std::size_t USpeakNative::USpeakFrameContainer::ReadContainer(std::vector<std::byte>& opusData, std::uint16_t& frameIndex, std::span<const std::byte> frametData)
{
    return ReadContainerImpl(opusData, frameIndex, frametData);
}

USpeakNative::USpeakFrameContainer::USpeakFrameContainer()
    : m_data()
{
}

std::size_t USpeakNative::USpeakFrameContainer::fromData(std::span<const std::byte> opusData, std::uint16_t frameIndex)
{
    return WriteContainerImpl(m_data, 0, opusData, frameIndex);
}

std::size_t USpeakNative::USpeakFrameContainer::decode(std::span<const std::byte> frameData)
{
    std::size_t frameSize = GetUSpeakFrameSize(frameData);

    // Read frame
    if (frameSize != 0) {
        m_data.assign(frameData.begin(), frameData.begin() + frameSize);
    }

    return frameSize;
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::encodedData() const noexcept
{
    return m_data;
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::decodedData() const noexcept
{
    if (m_data.size() <= USPEAKFRAME_HEADERSIZE) return {};

    return std::span<const std::byte>(m_data).subspan(USPEAKFRAME_HEADERSIZE);
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameIndex() const noexcept
{
    if (m_data.size() < USPEAKFRAME_HEADERSIZE) return 0;

    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data.data(), 0);
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameSize() const noexcept
{
    if (m_data.size() < USPEAKFRAME_HEADERSIZE) return 0;

    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data.data(), 2);
}
