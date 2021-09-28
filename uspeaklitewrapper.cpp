#include "uspeaklite.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdlib.h>
#include <string.h>

extern "C" __declspec(dllexport) USpeakNative::USpeakLite* Native_CreateUSpeakLite()
{
    return new USpeakNative::USpeakLite();
}

extern "C" __declspec(dllexport) void Native_DeleteUSpeakLite(USpeakNative::USpeakLite* ptr)
{
    delete ptr;
}

extern "C" __declspec(dllexport) bool Native_StreamMp3(USpeakNative::USpeakLite* ptr, const char* pathData, std::int32_t pathLength)
{
    if (pathData == nullptr || pathLength <= 0) {
        return false;
    }

    return ptr->streamFile(std::string_view(pathData, pathLength));
}

extern "C" __declspec(dllexport) std::int32_t Native_GetAudioFrame(USpeakNative::USpeakLite* ptr, std::int32_t playerId, std::int32_t packetTime, std::byte* buffer, std::int32_t bufferLength)
{
    if (buffer == nullptr || bufferLength <= 0) {
        return -1;
    }

    std::span<std::byte> bufferSpan(buffer, bufferLength);

    return ptr->getAudioFrame(playerId, packetTime, bufferSpan);
}
extern "C" __declspec(dllexport) std::int32_t Native_RecodeAudioFrame(USpeakNative::USpeakLite* ptr, std::byte* dataIn, std::int32_t dataInLength, std::byte* dataOut, std::int32_t dataOutLength)
{
    if (dataIn == nullptr || dataInLength <= 0 || dataOut == nullptr || dataOutLength <= 0) {
        return -1;
    }

    std::span<std::byte> dataInSpan(dataIn, dataInLength);

    auto bytes = ptr->recodeAudioFrame(dataInSpan);

    if (dataOutLength < (std::int32_t)bytes.size()) {
        return -1;
    }

    memcpy(dataOut, bytes.data(), bytes.size());

    return bytes.size();
}
