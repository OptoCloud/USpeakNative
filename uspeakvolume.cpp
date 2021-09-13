#include "uspeakvolume.h"

void USpeakNative::ApplyGain(std::span<float> samples, float gain)
{
    if (gain != 1.f) {
        for (std::size_t i = 0; i < samples.size(); i++) {
            samples[i] = std::min(std::max(samples[i] * gain, 1.f), -1.f);
        }
    }
}
