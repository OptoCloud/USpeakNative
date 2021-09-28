#include "uspeakframecontainer.h"

#include "helpers.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

constexpr std::size_t FrameHeaderSize = sizeof(std::uint16_t) + sizeof(std::uint16_t);

USpeakNative::USpeakFrameContainer::USpeakFrameContainer()
    : m_data(nullptr)
    , m_size(0)
{
}

USpeakNative::USpeakFrameContainer::~USpeakFrameContainer()
{
    if (m_data != nullptr) {
        free(m_data);
    }
}

bool USpeakNative::USpeakFrameContainer::fromData(std::span<const std::byte> data, std::uint16_t frameIndex)
{
    if (data.size() > UINT16_MAX || data.size() == 0) {
        return false;
    }

    if (!resize(data.size() + FrameHeaderSize)) {
        return false;
    }

    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(m_data, 0, frameIndex);
    USpeakNative::Helpers::ConvertToBytes<std::uint16_t>(m_data, 2, data.size());

    memcpy((std::byte*)m_data + FrameHeaderSize, data.data(), data.size());

    return true;
}

bool USpeakNative::USpeakFrameContainer::decode(std::span<const std::byte> data, std::size_t offset)
{
    const std::byte* actualData = data.data() + offset;
    std::size_t actualSize = data.size() - offset;

    if (actualSize < FrameHeaderSize) {
        fmt::print("Data size less then frame header size!\n");
        clear();
        return false;
    }

    if (actualSize < FrameHeaderSize) {
        fmt::print("Data size is less than frame header size!\n");
        clear();
        return false;
    }

    // Read frame
    if (!resize(actualSize)) {
        return false;
    }

    memcpy(m_data, actualData, actualSize);
    return true;
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::encodedData()
{
    auto begPtr = (const std::byte*)m_data;
    auto endPtr = (const std::byte*)m_data + m_size;

    return std::span<const std::byte>(begPtr, endPtr);
}

std::span<const std::byte> USpeakNative::USpeakFrameContainer::decodedData()
{
    auto begPtr = (const std::byte*)m_data + FrameHeaderSize;
    auto endPtr = (const std::byte*)m_data + m_size;

    return std::span<const std::byte>(begPtr, endPtr);
}

std::size_t USpeakNative::USpeakFrameContainer::encodedSize()
{
    return m_size;
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameIndex()
{
    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data, 0);
}

std::uint16_t USpeakNative::USpeakFrameContainer::frameSize()
{
    return USpeakNative::Helpers::ConvertFromBytes<std::uint16_t>(m_data, 2);
}

void USpeakNative::USpeakFrameContainer::clear()
{
    free(m_data);
    m_data = nullptr;
    m_size = 0;
}

bool USpeakNative::USpeakFrameContainer::resize(std::size_t size)
{
    if (size == 0) {
        free(m_data);
        m_size = 0;
        return true;
    }

    void* newPtr;
    if (m_data == nullptr) {
        newPtr = malloc(size);
    } else {
        newPtr = realloc(m_data, size);
        if (newPtr == nullptr) {
            free(m_data);
        }
    }

    m_data = newPtr;
    m_size = (m_data == nullptr) ? 0 : size;

    return m_data != nullptr;
}
