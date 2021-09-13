#include "uspeakresampler.h"

constexpr double sample_hermite_4p_3o(double x, double * y)
{
    double c0, c1, c2, c3;
    c0 = y[1];
    c1 = (1.0 / 2.0)*(y[2] - y[0]);
    c2 = (y[0] - (5.0 / 2.0)*y[1]) + (2.0*y[2] - (1.0 / 2.0)*y[3]);
    c3 = (1.0 / 2.0)*(y[3] - y[0]) + (3.0 / 2.0)*(y[1] - y[2]);
    return ((c3*x + c2)*x + c1)*x + c0;
}

void USpeakNative::Resample(std::span<const float> src, int srcSampleRate, std::vector<float>& dst, int dstSampleRate)
{
    // Get sample rate
    double rate = (double)srcSampleRate / (double)dstSampleRate;

    for (double virtualReadIndex = 0.; virtualReadIndex += rate;) {
        // Get aboslute readIndex
        std::uint32_t readIndex = static_cast<std::uint32_t>(virtualReadIndex);

        // If we have reached end, return
        if (readIndex + 4 >= src.size()) {
            return;
        }

        // Get fraction of overstep for readIndex
        double fraction = virtualReadIndex - readIndex;

        // Resample
        double samps[4] = { src[readIndex + 0], src[readIndex + 1], src[readIndex + 2], src[readIndex + 3] };
        double sample = sample_hermite_4p_3o(fraction, samps); // cubic hermite interpolate over 4 samples

        // Push back samples
        dst.push_back(static_cast<float>(sample));
    }
}
