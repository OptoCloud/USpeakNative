#ifndef USPEAK_USPEAKAUDIOGAIN_H
#define USPEAK_USPEAKAUDIOGAIN_H

#include <span>

namespace USpeakNative {

float GetRMS(std::span<const float> samples) noexcept;
void AutoLevel(std::span<float> samples, float rms, float rmsTarget, float& currentScale, float& runningScale) noexcept;
void ApplyGain(std::span<float> samples, float gain) noexcept;
void NormalizeGain(std::span<float> samples) noexcept;

}

#endif // USPEAK_USPEAKAUDIOGAIN_H
