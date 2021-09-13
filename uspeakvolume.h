#ifndef USPEAK_USPEAKAUDIOGAIN_H
#define USPEAK_USPEAKAUDIOGAIN_H

#include <span>

namespace USpeakNative {

void ApplyGain(std::span<float> samples, float gain);

}

#endif // USPEAK_USPEAKAUDIOGAIN_H
