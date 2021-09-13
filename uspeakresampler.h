#ifndef USPEAK_RESAMPLER_H
#define USPEAK_RESAMPLER_H

#include <span>
#include <vector>
#include <cstdint>

namespace USpeakNative {

void Resample(std::span<const float> src, int srcSampleRate, std::vector<float>& dst, int dstSampleRate);

}

#endif // USPEAK_RESAMPLER_H
