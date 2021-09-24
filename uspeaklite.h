#ifndef USPEAK_USPEAKLITE_H
#define USPEAK_USPEAKLITE_H

#include "uspeakframecontainer.h"
#include "opuscodec/opuscodec.h"
#include "opuscodec/bandmode.h"

#include <deque>
#include <memory>
#include <atomic>
#include <thread>

namespace USpeakNative {

namespace OpusCodec { class OpusCodec; }

class USpeakLite
{
public:
    USpeakLite();
    ~USpeakLite();

    int audioFrequency() const;
    USpeakNative::OpusCodec::BandMode bandMode() const;

    std::vector<std::uint8_t> getAudioFrame(std::uint32_t senderId, std::uint32_t packetTimer);
    std::vector<std::uint8_t> recodeAudioFrame(std::span<const std::uint8_t> packetTimer);
    bool streamFile(std::string_view filename);
private:
    void processingLoop();

    std::atomic_bool m_run;
    std::atomic_bool m_lock;
    std::shared_ptr<OpusCodec::OpusCodec> m_opusCodec;
    std::deque<USpeakNative::USpeakFrameContainer> m_frameQueue;
    std::thread m_processingThread;

    USpeakNative::OpusCodec::BandMode m_lastBandMode;
    USpeakNative::OpusCodec::BandMode m_bandMode;
    int m_recFreq;
    std::uint16_t m_ind;
};

}

#endif // USPEAK_USPEAKLITE_H
