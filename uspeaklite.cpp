#include "uspeaklite.h"

#include "helpers.h"
#include "uspeakvolume.h"
#include "uspeakresampler.h"

#include "fmt/core.h"
#include "libnyquist/Decoders.h"
#include "libnyquist/Encoders.h"
#include "internal/scopedspinlock.h"

constexpr std::size_t USPEAK_HEADERSIZE = sizeof(std::int32_t) + sizeof(std::int32_t);
constexpr std::size_t USPEAK_BUFFERSIZE = 1022;

struct PlayerData {
    int sampleIndex;
    std::uint32_t startTime;
    std::vector<float> framesToSave;
};

#include <mutex>
std::mutex uspeakPlayersLock;
std::unordered_map<std::int32_t, PlayerData> uspeakPlayers;

USpeakNative::USpeakLite::USpeakLite()
    : m_run(true)
    , m_lock(false)
    , m_opusCodec(std::make_shared<USpeakNative::OpusCodec::OpusCodec>(48000, 24000, USpeakNative::OpusCodec::OpusDelay::Delay_20ms))
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

std::size_t USpeakNative::USpeakLite::getAudioFrame(std::int32_t playerId, std::int32_t packetTime, std::span<std::byte> buffer)
{
    USpeakNative::Internal::ScopedSpinLock l(m_lock);

    if (m_frameQueue.empty() || buffer.size() < 1022) {
        return 0;
    }

    USpeakNative::Helpers::ConvertToBytes<std::int32_t>(buffer.data(), 0, playerId); // Set 4 bytes for playerId
    USpeakNative::Helpers::ConvertToBytes<std::int32_t>(buffer.data(), 4, packetTime); // Set 4 bytes for packetTime

    std::size_t sizeWritten = 8;

    for (int i = 0; i < 3; i++) {
        std::span<const std::byte> frameData = m_frameQueue.front().encodedData();

        if (sizeWritten + frameData.size() > buffer.size()) {
            break;
        }

        memcpy(buffer.data() + sizeWritten, frameData.data(), frameData.size());
        sizeWritten += frameData.size();

        m_frameQueue.pop();

        if (m_frameQueue.empty()) {
            break;
        }
    }

    return sizeWritten;
}

std::vector<std::byte> USpeakNative::USpeakLite::recodeAudioFrame(std::span<const std::byte> dataIn)
{
    if (dataIn.size() <= USPEAK_HEADERSIZE) {
        fmt::print("[USpeakNative] Audioframe too small!\n");
        return {};
    }

    // Get packet playerId and packetTime from first 8 bytes
    std::int32_t playerId = USpeakNative::Helpers::ConvertFromBytes<std::int32_t>(dataIn.data(), 0);
    std::int32_t packetTime = USpeakNative::Helpers::ConvertFromBytes<std::int32_t>(dataIn.data(), 4);

    // Get or create a uspeakplayer object to hold audio data from this player
    std::scoped_lock l(uspeakPlayersLock);
    auto it = uspeakPlayers.find(playerId);
    if (it == uspeakPlayers.end()) {
        fmt::print("[USpeakNative] Recording: uSpeaker[{}]\n", playerId);
        PlayerData data;
        data.sampleIndex = 0;
        data.startTime = packetTime;
        data.framesToSave.reserve(512000);
        it = uspeakPlayers.emplace(playerId, std::move(data)).first;
    }

    // Get all the audio packets, and decode them into float32 samples
    std::size_t offset = USPEAK_HEADERSIZE;
    USpeakNative::USpeakFrameContainer container;
    while (container.decode(dataIn, offset)) {

        auto hmm = m_opusCodec->decodeFloat(container.decodedData(), USpeakNative::OpusCodec::BandMode::Opus48k);

        offset += container.encodedData().size();

        it->second.framesToSave.insert(it->second.framesToSave.end(), hmm.begin(), hmm.end());
    }

    // If buffer has been filled, encode and save the audio data
    if (it->second.framesToSave.size() >= 500000) {

        // Server ticks are in ms so just calculate the time elapsed by getting the diff / 1000
        auto dur = (double)(packetTime - it->second.startTime) / 1000.;
        it->second.startTime = packetTime;

        fmt::print("[USpeakNative] Saving {} seconds from uSpeaker[{}]\n", dur, playerId);

        nqr::AudioData data;
        data.channelCount = 1;            // Single channel
        data.sampleRate = 24000;          // 24k bitrate
        data.sourceFormat = nqr::PCM_FLT; // PulseCodeModulation_FLoaT
        data.lengthSeconds = dur;         // amount of seconds elapsed
        data.frameSize = 32;              // bits per sample
        data.samples = it->second.framesToSave; // audio data

        // Encode and save to ogg files
        nqr::encode_opus_to_disk({ 1, nqr::PCM_FLT, nqr::DITHER_NONE }, &data, fmt::format("test-{}-{}.ogg", playerId, it->second.sampleIndex++));

        it->second.framesToSave.resize(0);
    }

    return std::vector<std::byte>(dataIn.begin(), dataIn.end());
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

        std::vector<float> swapBuffer;
        swapBuffer.reserve(fileData.samples.size());

        // Convert to mono
        if (fileData.channelCount == 2) {
            fmt::print("[USpeakNative] Converting to mono...\n");
            swapBuffer.resize(fileData.samples.size() / 2);

            nqr::StereoToMono(fileData.samples.data(), swapBuffer.data(), fileData.samples.size());

            std::swap(fileData.samples, swapBuffer);
            fileData.channelCount = 1;
        }

        // Resample to 24k
        if (fileData.sampleRate != 24000) {
            fmt::print("[USpeakNative] Resampling to 24k...\n");
            swapBuffer.resize(0);

            USpeakNative::Resample(fileData.samples, fileData.sampleRate, swapBuffer, 24000);

            std::swap(fileData.samples, swapBuffer);
            fileData.sampleRate = 24000;
        }

        std::size_t sampleSize = m_opusCodec->sampleSize();
        std::uint16_t frameIndex = 0;

        // Make sure the number of samples is a multiple of the codec sample size
        std::size_t nSamples = fileData.samples.size();
        std::size_t nSamplesFrames = nSamples / sampleSize;
        std::size_t wholeSampleFrames = nSamplesFrames * sampleSize;
        if (wholeSampleFrames != nSamples) {
            fileData.samples.resize(wholeSampleFrames + sampleSize);
        }

        // Adjust gain to keep sound between 1 and -1, but dont clip the audio
        fmt::print("[USpeakNative] Normalizing gain...\n");
        NormalizeGain(fileData.samples);

        // Encode
        fmt::print("[USpeakNative] Encoding...\n");
        auto it_a = fileData.samples.begin();
        auto it_end = fileData.samples.end();
        while (it_a != it_end) {
            auto it_b = it_a + sampleSize;

            USpeakNative::USpeakFrameContainer container;
            bool ok = container.fromData(m_opusCodec->encodeFloat(std::span<float>(it_a, it_b), USpeakNative::OpusCodec::BandMode::Opus48k), frameIndex++);

            if (ok) {
                m_frameQueue.push(std::move(container));
            }

            it_a = it_b;
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
