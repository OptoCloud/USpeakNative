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

extern "C" __declspec(dllexport) std::int32_t Native_GetAudioFrame(USpeakNative::USpeakLite* ptr, std::int32_t playerId, std::int32_t packetTime, std::byte* bufferPtr, std::int32_t bufferLength)
{
    if (bufferPtr == nullptr || bufferLength <= 0) {
        return -1;
    }

    std::span<std::byte> bufferSpan(bufferPtr, bufferLength);

    return ptr->getAudioFrame(playerId, packetTime, bufferSpan);
}
extern "C" __declspec(dllexport) std::int32_t Native_RecodeAudioFrame(USpeakNative::USpeakLite* ptr, std::byte* dataInPtr, std::int32_t dataInLength, std::byte* bufferPtr, std::int32_t bufferLength)
{
    if (dataInPtr == nullptr || dataInLength <= 0 || bufferPtr == nullptr || bufferLength <= 0) {
        return -1;
    }

    std::span<std::byte> dataInSpan(dataInPtr, dataInLength);

    auto bytes = ptr->recodeAudioFrame(dataInSpan);

    if (bufferLength < (std::int32_t)bytes.size()) {
        return -1;
    }

    memcpy(bufferPtr, bytes.data(), bytes.size());

    return bytes.size();
}
