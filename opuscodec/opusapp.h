#ifndef USPEAK_OPUSAPP_H
#define USPEAK_OPUSAPP_H

namespace USpeak::OpusCodec {

enum class OpusApp
{
    Voip = 2048,
    Audio,
    Restricted_LowLatency = 2051
};

}

#endif // USPEAK_OPUSAPP_H
