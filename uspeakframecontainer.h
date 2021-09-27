#ifndef USPEAK_USPEAKFRAMECONTAINER_H
#define USPEAK_USPEAKFRAMECONTAINER_H

#include <span>
#include <cstdint>

namespace USpeakNative {

struct USpeakFrameContainer
{
    USpeakFrameContainer();
    ~USpeakFrameContainer();
    bool fromData(std::span<const std::uint8_t> data, std::uint16_t frameIndex);
    bool decode(std::span<const std::uint8_t> data, std::size_t offset);

    std::span<const std::uint8_t> encodedData();
    std::span<const std::uint8_t> decodedData();

    std::size_t encodedSize();

    std::uint16_t frameIndex();
    std::uint16_t frameSize();
private:
    void clear();
    bool resize(std::size_t size);

    void* m_data;
    std::size_t m_size;
};

}

#endif // USPEAK_USPEAKFRAMECONTAINER_H
