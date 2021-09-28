#ifndef USPEAK_OPUSCODEC_H
#define USPEAK_OPUSCODEC_H

#include "opusdelay.h"
#include "bandmode.h"

#include <vector>
#include <array>
#include <span>

class OpusEncoder;
class OpusDecoder;

namespace USpeakNative::OpusCodec {

class OpusCodec
{
public:
    OpusCodec(int frequency, int bitrate, USpeakNative::OpusCodec::OpusDelay delay);
    ~OpusCodec();

    bool init();

    std::size_t sampleSize() noexcept;
    std::vector<std::byte> encodeFloat(std::span<const float> samples, USpeakNative::OpusCodec::BandMode mode);
    std::vector<float> decodeFloat(std::span<const std::byte> data, USpeakNative::OpusCodec::BandMode mode);
private:
    void destroyCodecs();

    OpusEncoder* m_encoder;
    OpusDecoder* m_decoder;
    int m_app;
    int m_frequency;
    int m_bitrate;
    int m_delay;
    std::size_t m_segmentFrames;
    std::array<std::byte, 1024> m_encodeBuffer;
    std::array<float, 4096> m_decodeBuffer;
};

}

#endif // USPEAK_OPUSCODEC_H
