#ifndef USPEAK_USPEAKPACKET_H
#define USPEAK_USPEAKPACKET_H

#include <vector>
#include <cstdint>

namespace USpeakNative {

struct USpeakPacket {
    std::int32_t playerId;
    std::uint32_t packetTime;
    std::vector<float> audioSamples;
};

}

#endif // USPEAK_USPEAKPACKET_H
