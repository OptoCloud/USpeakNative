#ifndef USPEAK_USPEAKFRAMECONTAINER_H
#define USPEAK_USPEAKFRAMECONTAINER_H

#include <span>
#include <vector>
#include <memory>

namespace USpeakNative {

struct USpeakFrameContainer
{
    USpeakFrameContainer();
    bool fromData(std::span<const std::uint8_t> data, std::uint16_t frameIndex);
    bool decode(std::span<const std::uint8_t> data, std::size_t offset);
    std::span<const std::uint8_t> encode();

    std::size_t encodedSize();

    std::uint16_t frameSize();
    std::uint16_t frameIndex();
private:
    std::uint16_t m_frameSize;
    std::uint16_t m_frameIndex;
    std::vector<std::uint8_t> m_frameData;
};

}

#endif // USPEAK_USPEAKFRAMECONTAINER_H
