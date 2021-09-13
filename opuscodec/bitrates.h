#ifndef USPEAK_BITRATES_H
#define USPEAK_BITRATES_H

namespace USpeak::OpusCodec {

enum class Bitrates
{
    BitRate_8K = 8000,
    BitRate_10K = 10000,
    BitRate_16K = 16000,
    BitRate_18K = 18000,
    BitRate_20K = 20000,
    BitRate_24K = 24000,
    BitRate_32K = 32000,
    BitRate_48K = 48000,
    BitRate_64k = 64000,
    BitRate_96k = 96000,
    BitRate_128k = 128000,
    BitRate_256k = 256000,
    BitRate_384k = 384000,
    BitRate_512k = 512000
};

}

#endif // USPEAK_BITRATES_H
