#include "opuscodec.h"

#include "opuserror.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <opus/opus.h>

USpeakNative::OpusCodec::OpusCodec::OpusCodec(int frequency, int bitrate, int delay)
    : m_encoder(nullptr)
    , m_decoder(nullptr)
    , m_app(OPUS_APPLICATION_VOIP)
    , m_frequency(frequency)
    , m_bitrate(bitrate)
    , m_delay(delay)
    , m_segmentFrames(delay * (frequency / 1000))
    , m_encodeBuffer(4000, 0)
    , m_decodeBuffer(1000, 0.f)
{
}

USpeakNative::OpusCodec::OpusCodec::~OpusCodec()
{
    destroyCodecs();
}

bool USpeakNative::OpusCodec::OpusCodec::init()
{
    int err;
    m_encoder = opus_encoder_create(m_frequency, 1, m_app, &err);
    if (err != OPUS_OK) {
        destroyCodecs();
        return false;
    }
    err = opus_encoder_ctl(m_encoder, OPUS_SET_BITRATE(m_bitrate));
    if (err != OPUS_OK) {
        destroyCodecs();
        return false;
    }
    err = opus_encoder_ctl(m_encoder, OPUS_SET_INBAND_FEC(false));
    if (err != OPUS_OK) {
        destroyCodecs();
        return false;
    }
    err = opus_encoder_ctl(m_encoder, OPUS_SET_PACKET_LOSS_PERC(0));
    if (err != OPUS_OK) {
        destroyCodecs();
        return false;
    }

    m_decoder = opus_decoder_create(m_frequency, 1, &err);
    if (err != OPUS_OK) {
        destroyCodecs();
        return false;
    }

    return true;
}

std::size_t USpeakNative::OpusCodec::OpusCodec::sampleSize()
{
    return m_segmentFrames;
}

std::vector<std::uint8_t> USpeakNative::OpusCodec::OpusCodec::encodeFloat(std::span<const float> samples, USpeakNative::OpusCodec::BandMode mode)
{
    if (mode != USpeakNative::OpusCodec::BandMode::Opus48k) {
        fmt::print("[USpeakNative] OpusCodec: Encode: bandwidth mode must be {}! (set to {})\n",
                   USpeakNative::OpusCodec::BandModeString(USpeakNative::OpusCodec::BandMode::Opus48k),
                   USpeakNative::OpusCodec::BandModeString(mode));
        return {};
    }

    if (samples.size() != m_segmentFrames) {
        fmt::print("[USpeakNative] OpusCodec: Encode failed! Input PCM data is {} frames, expected {}\n",
                   samples.size(),
                   m_segmentFrames);
        return {};
    }

    int num = opus_encode_float(m_encoder, samples.data(), (int)samples.size(), m_encodeBuffer.data(), (int)m_encodeBuffer.size());
    if (num < 0) {
        fmt::print("[USpeakNative] OpusCodec: Encode failed! Opus Error_{}\n", num);
        return {};
    }
    if (num == 0) {
        fmt::print("[USpeakNative] OpusCodec: Encode failed! Nothing encoded...\n");
        return {};
    }

    return std::vector<std::uint8_t>(m_encodeBuffer.begin(), m_encodeBuffer.begin() + num);
}

std::vector<float> USpeakNative::OpusCodec::OpusCodec::decodeFloat(std::span<const std::uint8_t> data, USpeakNative::OpusCodec::BandMode mode)
{
    if (mode != USpeakNative::OpusCodec::BandMode::Opus48k) {
        fmt::print("[USpeakNative] OpusCodec: Decode: bandwidth mode must be {}! (set to {})\n",
                   USpeakNative::OpusCodec::BandModeString(USpeakNative::OpusCodec::BandMode::Opus48k),
                   USpeakNative::OpusCodec::BandModeString(mode));
        return {};
    }

    int num = opus_decode_float(m_decoder, data.data(), (int)data.size(), m_decodeBuffer.data(), (int)m_decodeBuffer.size(), 0);
    if (num < 0) {
        fmt::print("[USpeakNative] OpusCodec: Decode failed! Opus Error_{}\n", num);
        return {};
    }
    if (num == 0) {
        fmt::print("[USpeakNative] OpusCodec: Decode failed! Nothing decoded...\n");
        return {};
    }

    return std::vector<float>(m_decodeBuffer.begin(), m_decodeBuffer.begin() + num);
}

void USpeakNative::OpusCodec::OpusCodec::destroyCodecs()
{
    if (m_encoder != nullptr) {
        opus_encoder_destroy(m_encoder);
        m_encoder = nullptr;
    }
    if (m_decoder != nullptr) {
        opus_decoder_destroy(m_decoder);
        m_decoder = nullptr;
    }
}
