#ifndef USPEAK_BANDMODE_H
#define USPEAK_BANDMODE_H

#include <string_view>

namespace USpeakNative::OpusCodec {

enum class BandMode
{
    Narrow = 0,
    Wide = 1,
    UltraWide = 2,
    Opus48k = 3
};

constexpr std::string_view BandModeString(USpeakNative::OpusCodec::BandMode bandmode) noexcept {
    using namespace std::literals;
    switch (bandmode) {
    case USpeakNative::OpusCodec::BandMode::Narrow:
        return "BandMode::Narrow"sv;
    case USpeakNative::OpusCodec::BandMode::Wide:
        return "BandMode::Wide"sv;
    case USpeakNative::OpusCodec::BandMode::UltraWide:
        return "BandMode::UltraWide"sv;
    case USpeakNative::OpusCodec::BandMode::Opus48k:
        return "BandMode::Opus48k"sv;
    default:
        return "BandMode::Unkown"sv;
    }
}
constexpr std::uint32_t BandModeFrequency(USpeakNative::OpusCodec::BandMode bandmode) noexcept {
    switch (bandmode) {
    case USpeakNative::OpusCodec::BandMode::Narrow:
        return 8000;
    case USpeakNative::OpusCodec::BandMode::Wide:
        return 16000;
    case USpeakNative::OpusCodec::BandMode::UltraWide:
        return 32000;
    case USpeakNative::OpusCodec::BandMode::Opus48k:
        return 48000;
    default:
        return 0;
    }
}

}

#endif // USPEAK_BANDMODE_H
