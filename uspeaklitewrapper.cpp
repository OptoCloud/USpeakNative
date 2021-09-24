#ifndef USPEAKNATIVEWRAPPER_H
#define USPEAKNATIVEWRAPPER_H

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

extern "C" __declspec(dllexport) std::int32_t Native_GetAudioFrame(USpeakNative::USpeakLite* ptr, void* data, std::int32_t dataLength, std::uint32_t senderId, std::uint32_t packetTime)
{
    if (data == nullptr || dataLength <= 0) {
        return -1;
    }

    auto bytes = ptr->getAudioFrame(senderId, packetTime);

    if (dataLength < (std::int32_t)bytes.size()) {
        return -1;
    }

    memcpy(data, bytes.data(), bytes.size());

    return bytes.size();
}
extern "C" __declspec(dllexport) std::int32_t Native_RecodeAudioFrame(USpeakNative::USpeakLite* ptr, void* dataIn, std::int32_t dataInLength, void* dataOut, std::int32_t dataOutLength)
{
    if (dataIn == nullptr || dataInLength <= 0 || dataOut == nullptr || dataOutLength <= 0) {
        return -1;
    }

    auto bytes = ptr->recodeAudioFrame(std::span<std::uint8_t>((std::uint8_t*)dataIn, dataInLength));

    if (dataOutLength < (std::int32_t)bytes.size()) {
        return -1;
    }

    memcpy(dataOut, bytes.data(), bytes.size());

    return bytes.size();
}

#endif // USPEAKNATIVEWRAPPER_H
