#include "uspeakvolume.h"

void USpeakNative::ApplyGain(std::span<float> samples, float gain) noexcept
{
    if (gain != 1.f) {
        for (std::size_t i = 0; i < samples.size(); i++) {
            samples[i] = std::min(std::max(samples[i] * gain, 1.f), -1.f);
        }
    }
}

void USpeakNative::NormalizeGain(std::span<float> samples) noexcept
{
    float maxgain = 0.f;
    for (std::size_t i = 0; i < samples.size(); i++) {
        maxgain = std::max(std::abs(samples[i]), maxgain);
    }
    if (maxgain > 1.f) {
        float ratio = 1.f / maxgain;
        for (std::size_t i = 0; i < samples.size(); i++) {
            samples[i] = samples[i] * ratio;
        }
    }
}
