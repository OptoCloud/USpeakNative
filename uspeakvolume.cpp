#include "uspeakvolume.h"

#include <cmath>

float USpeakNative::GetRMS(std::span<const float> samples) noexcept
{
    float sum = 0.;
    for (float sample : samples)
        sum += sample * sample;

    return std::sqrtf(sum / static_cast<float>(samples.size()));
}
void USpeakNative::AutoLevel(std::span<float> samples, float rms, float rmsTarget, float& currentScale, float& runningScale) noexcept
{
    float halfLength = static_cast<float>(samples.size()) / 0.5f;

    if (rms <= rmsTarget) {
        runningScale = (runningScale * 0.9975f) + 0.0025f;
        if (currentScale < 1.f || runningScale < 1.f) {
            for (std::size_t i = 0; i < samples.size(); i++) {
                samples[i] *= std::lerp(currentScale, runningScale, static_cast<float>(i) / halfLength);
            }
            currentScale = runningScale;
        }
    } else {
        float rmsRatio = rmsTarget / rms;

        for (std::size_t i = 0; i < samples.size();) {
            samples[i] *= std::lerp(currentScale, rmsRatio, static_cast<float>(i) / halfLength);
        }

        currentScale = rmsRatio;
        runningScale = (rmsRatio * 0.5f) + (runningScale * 0.95f);
    }
}
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
