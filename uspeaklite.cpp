#include "uspeaklite.h"

#include "uspeakvolume.h"
#include "uspeakresampler.h"

#include "fmt/core.h"
#include "libnyquist/Decoders.h"
#include "libnyquist/Encoders.h"
#include "internal/scopedspinlock.h"

constexpr std::size_t USPEAK_BUFFERSIZE = 1022;

constexpr void SetPacketId(std::span<std::uint8_t> packet, std::uint32_t packetId) {
    *((std::int32_t*)(packet.data() + 0)) = packetId;
}
constexpr std::uint32_t GetPacketSenderId(std::span<const std::uint8_t> packet) {
    return *((std::int32_t*)(packet.data() + 0));
}
constexpr void SetPacketServerTime(std::span<std::uint8_t> packet, std::uint32_t packetTime) {
   *((std::int32_t*)(packet.data() + 4)) = packetTime;
}
constexpr std::uint32_t GetPacketServerTime(std::span<const std::uint8_t> packet) {
    return *((std::int32_t*)(packet.data() + 4));
}

struct PlayerData {
    int sampleIndex;
    std::uint32_t startTicks;
    std::vector<float> framesToSave;
};

#include <mutex>
std::mutex uspeakPlayersLock;
std::unordered_map<std::uint8_t, PlayerData> uspeakPlayers;

USpeakNative::USpeakLite::USpeakLite()
    : m_run(true)
    , m_lock(false)
    , m_opusCodec(std::make_shared<USpeakNative::OpusCodec::OpusCodec>())
    , m_frameQueue()
    , m_processingThread(&USpeakNative::USpeakLite::processingLoop, this)
    , m_lastBandMode()
    , m_bandMode(USpeakNative::OpusCodec::BandMode::Opus48k)
    , m_recFreq(USpeakNative::OpusCodec::BandModeFrequency(m_bandMode))
    , m_ind(0)
{
    fmt::print("[USpeakNative] Made by OptoCloud\n");
    if (!m_opusCodec->init()) {
        throw std::exception("Failed to initialize codec!");
    }
    fmt::print("[USpeakNative] Initialized!\n");
}

USpeakNative::USpeakLite::~USpeakLite()
{
    m_run.store(false, std::memory_order::relaxed);
    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }

    fmt::print("[USpeakNative] Destroyed!\n");
}

int USpeakNative::USpeakLite::audioFrequency() const
{
    return m_recFreq;
}

USpeakNative::OpusCodec::BandMode USpeakNative::USpeakLite::bandMode() const
{
    return m_bandMode;
}

std::vector<std::uint8_t> USpeakNative::USpeakLite::getAudioFrame(std::uint32_t actorNr, std::uint32_t packetTime)
{
    USpeakNative::Internal::ScopedSpinLock l(m_lock);

    if (m_frameQueue.empty()) {
        return {};
    }

    std::vector<std::uint8_t> buffer;
    buffer.reserve(USPEAK_BUFFERSIZE);
    buffer.resize(8);

    while (m_frameQueue.size() < 3) {}
    for (int i = 0; i < 3; i++) {
        USpeakFrameContainer& frame = m_frameQueue.front();

        auto frameData = frame.encodedData();

        buffer.insert(buffer.end(), frameData.begin(), frameData.end());

        m_frameQueue.pop_front();
    }

    SetPacketId(buffer, actorNr);
    SetPacketServerTime(buffer, packetTime);

    return buffer;
}

std::vector<std::uint8_t> USpeakNative::USpeakLite::recodeAudioFrame(std::span<const std::uint8_t> dataIn)
{
    std::uint32_t senderId = GetPacketSenderId(dataIn);
    std::uint32_t serverTicks = GetPacketServerTime(dataIn);

    std::scoped_lock l(uspeakPlayersLock);
    auto it = uspeakPlayers.find(senderId);
    if (it == uspeakPlayers.end()) {
        fmt::print("[USpeakNative] Recording: uSpeaker[{}]\n", senderId);
        PlayerData data;
        data.sampleIndex = 0;
        data.startTicks = serverTicks;
        data.framesToSave.reserve(512000);
        it = uspeakPlayers.emplace(senderId, std::move(data)).first;
    }

    std::size_t offset = 8;
    while (offset < dataIn.size()) {
        USpeakNative::USpeakFrameContainer container;
        if (!container.decode(dataIn, offset)) {
            fmt::print("[USpeakNative] Failed to decode audio packet!\n");
            return {};
        }

        auto hmm = m_opusCodec->decodeFloat(container.decodedData(), USpeakNative::OpusCodec::BandMode::Opus48k);

        offset += container.encodedSize();

        it->second.framesToSave.insert(it->second.framesToSave.end(), hmm.begin(), hmm.end());
    }

    if (it->second.framesToSave.size() >= 500000) {
        auto dur = (double)(serverTicks - it->second.startTicks) / 1000.;
        it->second.startTicks = serverTicks;

        fmt::print("[USpeakNative] Saving {} seconds from uSpeaker[{}]\n", dur, senderId);

        nqr::AudioData data;
        data.channelCount = 1;
        data.sampleRate = 16000;
        data.lengthSeconds = dur;
        data.frameSize = 1 * 8;
        data.samples = it->second.framesToSave;
        data.sourceFormat = nqr::PCM_FLT;

        nqr::EncoderParams params;
        params.channelCount = 1;
        params.targetFormat = nqr::PCM_FLT;

        nqr::encode_wav_to_disk(params, &data, fmt::format("test-{}-{}.wav", senderId, it->second.sampleIndex++));

        it->second.framesToSave.resize(0);
    }

    return std::vector<std::uint8_t>(dataIn.begin(), dataIn.end());
}

bool USpeakNative::USpeakLite::streamFile(std::string_view filename)
{
    fmt::print("[USpeakNative] Loading: {}\n", filename);

    USpeakNative::Internal::ScopedSpinLock l(m_lock);

    try {
        nqr::NyquistIO loader;
        nqr::AudioData fileData;

        loader.Load(&fileData, std::string(filename));

        if (fileData.channelCount == 0 || fileData.channelCount > 2) {
            fmt::print("[USpeakNative] Invalid channelcount: {}\n", fileData.channelCount);
            return false;
        }

        // Convert to mono
        if (fileData.channelCount == 2) {
            fmt::print("[USpeakNative] Converting to mono...\n");
            std::vector<float> mono(fileData.samples.size() / 2);

            nqr::StereoToMono(fileData.samples.data(), mono.data(), fileData.samples.size());

            fileData.samples = std::move(mono);
            fileData.channelCount = 1;
        }

        // Resample to 24k
        if (fileData.sampleRate != 24000) {
            fmt::print("[USpeakNative] Resampling to 24k...\n");
            std::vector<float> resampled;
            resampled.reserve(fileData.samples.size());

            USpeakNative::Resample(fileData.samples, fileData.sampleRate, resampled, 24000);

            fileData.samples = std::move(resampled);
            fileData.sampleRate = 24000;
        }

        std::uint16_t frameIndex = 0;
        std::size_t sampleSize = m_opusCodec->sampleSize();

        std::size_t i_beg = 0;
        for (;;) {
            if (i_beg == fileData.samples.size()) {
                break;
            }
            std::size_t i_end = std::min(i_beg + sampleSize, fileData.samples.size());

            auto itb = fileData.samples.begin() + i_beg;
            auto ite = fileData.samples.begin() + i_end;

            USpeakNative::USpeakFrameContainer container;

            bool ok = container.fromData(m_opusCodec->encodeFloat(std::span<float>(itb, ite), USpeakNative::OpusCodec::BandMode::Opus48k), frameIndex++);

            if (ok) {
                m_frameQueue.push_back(std::move(container));
            }

            i_beg = i_end;
        }

        fmt::print("[USpeakNative] Loaded!\n");
    } catch (const std::exception& ex) {
        fmt::print("[USpeakNative] Failed to read file: {}\n", ex.what());
        return false;
    } catch (const std::string& ex) {
        fmt::print("[USpeakNative] Failed to read file: {}\n", ex);
        return false;
    } catch (const char* ex) {
        fmt::print("[USpeakNative] Failed to read file: {}\n", ex);
        return false;
    } catch (...) {
        fmt::print("[USpeakNative] Failed to read file: Unknown error\n");
        return false;
    }

    return true;
}

void USpeakNative::USpeakLite::processingLoop()
{

}
