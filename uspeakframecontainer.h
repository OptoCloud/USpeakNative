#ifndef USPEAK_USPEAKFRAMECONTAINER_H
#define USPEAK_USPEAKFRAMECONTAINER_H

#include <span>
#include <vector>
#include <cstdint>

namespace USpeakNative {

struct USpeakFrameContainer
{
    static std::size_t WriteContainer(std::vector<std::byte>& frameData, std::size_t frameDataOffset, std::span<const std::byte> opusData, std::uint16_t frameIndex);
    static std::size_t ReadContainer(std::vector<std::byte>& opusData, std::uint16_t& frameIndex, std::span<const std::byte> frameData);

    USpeakFrameContainer();

    std::size_t fromData(std::span<const std::byte> opusData, std::uint16_t frameIndex);
    std::size_t decode(std::span<const std::byte> frameData);

    std::span<const std::byte> encodedData() const noexcept;
    std::span<const std::byte> decodedData() const noexcept;

    std::uint16_t frameIndex() const noexcept;
    std::uint16_t frameSize() const noexcept;
private:
    std::vector<std::byte> m_data;
};

}

#endif // USPEAK_USPEAKFRAMECONTAINER_H
