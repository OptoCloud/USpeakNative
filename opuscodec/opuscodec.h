#ifndef USPEAK_OPUSCODEC_H
#define USPEAK_OPUSCODEC_H

#include "opusdelay.h"
#include "bandmode.h"

#include <vector>
#include <span>

class OpusEncoder;
class OpusDecoder;

namespace USpeakNative::OpusCodec {

class OpusCodec
{
public:
    OpusCodec(int frequency = 48000, int bitrate = 24000, int delay = 20);
    ~OpusCodec();

    bool init();

    std::size_t sampleSize();
    std::vector<std::uint8_t> encodeFloat(std::span<const float> samples, USpeakNative::OpusCodec::BandMode mode);
    std::vector<float> decodeFloat(std::span<const std::uint8_t> data, USpeakNative::OpusCodec::BandMode mode);
private:
    void destroyCodecs();

    OpusEncoder* m_encoder;
    OpusDecoder* m_decoder;
    int m_app;
    int m_frequency;
    int m_bitrate;
    int m_delay;
    std::size_t m_segmentFrames;
    std::vector<std::uint8_t> m_encodeBuffer;
    std::vector<float> m_decodeBuffer;
};

}

#endif // USPEAK_OPUSCODEC_H
