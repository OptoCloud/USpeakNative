#ifndef USPEAK_USPEAKFRAMECONTAINER_H
#define USPEAK_USPEAKFRAMECONTAINER_H

#include <span>
#include <vector>
#include <cstdint>

namespace USpeakNative {

struct USpeakFrameContainer
{
    USpeakFrameContainer();

    bool fromData(std::span<const std::byte> data, std::uint16_t frameIndex);
    bool decode(std::span<const std::byte> data, std::size_t offset);

    std::span<const std::byte> encodedData() const;
    std::span<const std::byte> decodedData() const;

    std::uint16_t frameIndex() const;
    std::uint16_t frameSize() const;
private:
    std::vector<std::byte> m_data;
};

}

#endif // USPEAK_USPEAKFRAMECONTAINER_H
