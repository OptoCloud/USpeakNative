#include "nlohmann/json.hpp"
#include "libnyquist/Decoders.h"
#include "libnyquist/Encoders.h"
#include "uspeaklite.h"
#include "base64.h"

#include <iostream>
#include <fstream>

int main(int argc, char**argv) {
    if (argc != 2) {
        printf("Usage: USpeakTest.exe [path_to_photon_log]");
        return EXIT_FAILURE;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs.is_open()) {
        printf("failed to open logfile!");
        return EXIT_FAILURE;
    }


    USpeakNative::USpeakLite uSpeak;
    USpeakNative::USpeakPacket uSpeakPacket;
    std::vector<std::byte> rawData;

    nqr::AudioData data;
    int i = 0;

    auto json = nlohmann::json::parse(ifs);
    for (const auto& entry : json) {
        if (entry["patch_name"] != "OnEventPatch") continue;

        auto& eventData = entry["patch_args"]["eventData"];
        if (eventData["Code"] != 1.f) continue;

        auto& customData = eventData["CustomData"];
        if (customData["type"] != "System.Byte[]") continue;

        auto err = macaron::Base64::Decode(customData["data"], rawData);
        if (err != "") {
            printf("Error: %s\n", err.c_str());
            continue;
        }

        if (!uSpeak.decodePacket(rawData, uSpeakPacket)) {
            printf("Decoding error!\n");
        }

        data.samples.insert(data.samples.end(), uSpeakPacket.pcmSamples.begin(), uSpeakPacket.pcmSamples.end());
        i++;
    }

    nqr::EncoderParams params;
    params.channelCount = 2;
    params.dither = nqr::DitherType::DITHER_NONE;
    params.targetFormat = nqr::PCMFormat::PCM_FLT;

    data.channelCount = 2;
    data.lengthSeconds = i * 0.01f;
    data.sampleRate = 48000;
    data.sourceFormat = nqr::PCMFormat::PCM_FLT;

    nqr::encode_opus_to_disk(params, &data, "test.ogg");
}
