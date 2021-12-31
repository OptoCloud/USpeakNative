#include "nlohmann/json.hpp"
#include "libnyquist/Decoders.h"
#include "libnyquist/Encoders.h"
#include "uspeaklite.h"
#include "base64.h"

#include <iostream>
#include <fstream>

void InsertAudio(std::vector<float>& target, std::uint32_t posMs, std::span<const float> src) {
    std::uint32_t targetIndex = posMs * 48;

    std::size_t requiredSpace = targetIndex + src.size();
    if (target.size() < requiredSpace) {
        target.resize(requiredSpace);
    }

    for (std::uint32_t i = 0; i < src.size(); i++) {
        target[targetIndex + i] = (target[targetIndex + i] + src[i]) / 2.f;
    }
}

bool writeDiff(const std::string& name, const std::vector<std::byte>& data) {
    std::fstream diff_fs("uspeak_diff_" + name + ".bin", std::ios::out | std::ios::binary);
    if (!diff_fs.is_open()) return false;
    diff_fs.write((const char*)data.data(), data.size());
    diff_fs.close();

    return true;
}

template<typename T, int Radius>
static inline T Lanczos(T x)
{
    if (x == 0.0) return 1.0;
    if (x <= -Radius || x >= Radius) return 0.0;
    T pi_x = x * 3.14159265358979323846f;
    return Radius * std::sin(pi_x) * std::sin(pi_x / Radius) / (pi_x * pi_x);
}

template<typename T, int FilterRadius>
static inline void Resample(std::span<const T> src, std::span<T> dest)
{
    const T blur = 1.0;
    const T factor = dest.size() / (T)src.size();

    T scale   = std::min(factor, 1.0f) / blur;
    T support = FilterRadius / scale;

    std::vector<T> contribution(std::min(src.size(), 5+size_t(2*support)));

    if (support <= 0.5f) {
        support = 0.5f + 1E-12; scale = 1.0f;
    }

    for (size_t x=0; x<dest.size(); ++x) {
        T center = (x+0.5f) / factor;
        size_t start = (size_t)std::max(center-support+0.5f, (T)0);
        size_t stop  = (size_t)std::min(center+support+0.5f, (T)src.size());
        T density = 0.0f;
        size_t nmax = stop-start;
        T s = start - center+0.5f;
        dest[x] = 0;
        for (size_t n=0; n<nmax; ++n, ++s) {
            contribution[n] = Lanczos<T, FilterRadius>(s * scale);
            density += contribution[n];
            dest[x] += src[start+n]*contribution[n];
        }
        if (density != 0.0 && density != 1.0) {
            dest[x] /= density;
        }
    }
}

template<typename T>
struct MinMax {
    T min;
    T max;
};
template<typename T>
MinMax<T> GetMinMax(std::span<T> data) {
    MinMax<T> mm;
    for (T entry : data) {
        if (entry < mm.min) {
            mm.min = entry;
        } else if (entry > mm.max) {
            mm.max = entry;
        }
    }
    return mm;
}

template<typename T, std::size_t ScaleX, std::size_t ScaleY>
void PrintGraph(std::span<T> data) {
    std::array<T, ScaleX> gdat;
    Resample<T, 3>(data, gdat);

    MinMax<T> mm = GetMinMax<T>(data);

    T stepSize = static_cast<T>(1) / static_cast<T>(ScaleY);

    printf("\nGraph:\n");
    for (int y = 0; y < ScaleY; y++) {
        for (int x = 0; x < ScaleX; x++) {
            if (y == 0 || y == (ScaleY - 1)) {
                putchar('-');
                if (x == (ScaleX - 1)) {
                    printf(" %f", y == 0 ? mm.max : mm.min);
                }
            } else {
                if (gdat[x] >= std::lerp(mm.max, mm.min, stepSize * static_cast<T>(y))) {
                    putchar('#');
                } else {
                    putchar(' ');
                }
            }
        }
        putchar('\n');
    }
    putchar('\n');
}

template <typename T, std::size_t StatSize>
class AudioMeaner {
public:
    AudioMeaner() {
        Init();
    }
    void Init() {
        for (std::size_t i = 0; i < StatSize; i++) {
            m_samplesAccum[i] = static_cast<T>(0);
        }
        m_samplesTaken = 0;
    }
    void Add(std::span<T> data) {
        std::array<T, StatSize> result;
        Resample<T, 3>(data, result);
        for (std::size_t i = 0; i < StatSize; i++) {
            m_samplesAccum[i] += result[i];
        }
        m_samplesTaken++;
    }
    std::array<T, StatSize> GetMean() {
        std::array<T, StatSize> mean;
        for (std::size_t i = 0; i < StatSize; i++) {
            mean[i] = (m_samplesAccum[i] / static_cast<T>(m_samplesTaken));
        }
        return mean;
    }
private:
    std::size_t m_samplesTaken;
    std::array<T, StatSize> m_samplesAccum;
};

int main(int argc, char**argv) {
    if (argc != 2) {
        printf("Usage: USpeakTest.exe [path_to_photon_log]");
        return EXIT_FAILURE;
    }

    AudioMeaner<float, 200> meaner;

    std::fstream ifs(argv[1], std::ios::in);
    if (!ifs.is_open()) {
        printf("failed to open logfile!");
        return EXIT_FAILURE;
    }

    USpeakNative::USpeakLite uSpeak;

    std::uint32_t startMs = UINT32_MAX;
    std::uint32_t endMs = 0;
    std::vector<std::byte> rawData;
    std::vector<USpeakNative::USpeakPacket> packets;

    // std::int32_t firstId = 0;

    auto json = nlohmann::json::parse(ifs);
    for (const auto& entry : json) {
        if (entry["patch_name"] != "OnEventPatch") continue;

        auto& eventData = entry["patch_args"]["eventData"];
        if (eventData["Code"] != 1.f) continue;

        auto& customData = eventData["CustomData"];
        if (customData["type"] != "System.Byte[]") continue;

        auto err = macaron::Base64::Decode(customData["data"], rawData);
        if (err != "") {
            printf("Error: %s\n", err.c_str());
            continue;
        }

        USpeakNative::USpeakPacket packet;
        if (!uSpeak.decodePacket(rawData, packet)) {
            printf("Decoding error!\n");
            return EXIT_FAILURE;
        }

        PrintGraph<float, 200, 80>(packet.audioSamples);
        meaner.Add(packet.audioSamples);
/*
        std::vector<std::byte> reEncoded;
        if (!uSpeak.encodePacket(packet, reEncoded)) {
            printf("Encoding error!\n");
            return EXIT_FAILURE;
        }
        if (rawData != reEncoded) {
            printf("DIFFERENT, saving diff...\n");

            std::fstream diff_fs("uspeak_diff.bin", std::ios::out | std::ios::binary);
            if (!writeDiff("a", rawData) || !writeDiff("b", reEncoded)) {
                printf("failed to create diff file!");
            }
            return EXIT_FAILURE;
        }*/
/*
        if (firstId == 0) {
            firstId = packet.playerId;
            printf("PlayerID: %i\n", firstId);
        } else if (packet.playerId != firstId) continue;
*/
        if (packet.packetTime < startMs) {
            startMs = packet.packetTime;
        } else if (packet.packetTime > endMs) {
            endMs = packet.packetTime;
        }

        packets.push_back(std::move(packet));
    }

    auto mean = meaner.GetMean();
    PrintGraph<float, 200, 80>(mean);

    nqr::AudioData data;
    data.samples.reserve(5000000);

    for (const auto& packet : packets) {
        InsertAudio(data.samples, packet.packetTime - startMs, packet.audioSamples);
    }

    nqr::EncoderParams params;
    params.channelCount = 1;
    params.dither = nqr::DitherType::DITHER_NONE;
    params.targetFormat = nqr::PCMFormat::PCM_FLT;

    data.channelCount = 1;
    data.lengthSeconds = (float)(endMs - startMs) / 1000.f;
    data.sampleRate = 48000;
    data.sourceFormat = nqr::PCMFormat::PCM_FLT;

    printf("Length is %f seconds!\n", data.lengthSeconds);
    fflush(stdout);

    nqr::encode_opus_to_disk(params, &data, "test.ogg");
}
