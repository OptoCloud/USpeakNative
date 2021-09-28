#ifndef USPEAK_USPEAKLITE_H
#define USPEAK_USPEAKLITE_H

#include "uspeakframecontainer.h"
#include "opuscodec/opuscodec.h"
#include "opuscodec/bandmode.h"

#include <span>
#include <queue>
#include <memory>
#include <atomic>
#include <thread>
#include <cstdint>

namespace USpeakNative {

namespace OpusCodec { class OpusCodec; }

class USpeakLite
{
public:
    USpeakLite();
    ~USpeakLite();

    int audioFrequency() const;
    USpeakNative::OpusCodec::BandMode bandMode() const;

    std::size_t getAudioFrame(std::int32_t playerId, std::int32_t packetTime, std::span<std::byte> buffer);
    std::vector<std::byte> recodeAudioFrame(std::span<const std::byte> packetTimer);
    bool streamFile(std::string_view filename);
private:
    void processingLoop();

    std::atomic_bool m_run;
    std::atomic_bool m_lock;
    std::shared_ptr<OpusCodec::OpusCodec> m_opusCodec;
    std::queue<USpeakFrameContainer> m_frameQueue;
    std::thread m_processingThread;

    USpeakNative::OpusCodec::BandMode m_lastBandMode;
    USpeakNative::OpusCodec::BandMode m_bandMode;
    int m_recFreq;
    std::uint16_t m_ind;
};

}

#endif // USPEAK_USPEAKLITE_H
