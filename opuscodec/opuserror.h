#ifndef USPEAK_OPUSERROR_H
#define USPEAK_OPUSERROR_H

#include <string_view>

namespace USpeakNative::OpusCodec {

enum class OpusError
{
    Ok = 0,
    BadArg = -1,
    BufferToSmall = -2,
    InternalError = -3,
    InvalidPacket = -4,
    Unimplemented = -5,
    InvalidState = -6,
    AllocFail = -7
};

constexpr std::string_view OpusErrorString(USpeakNative::OpusCodec::OpusError error) {
    using namespace std::literals;
    switch (error) {
    case USpeakNative::OpusCodec::OpusError::Ok:
        return "OpusError::Ok"sv;
    case USpeakNative::OpusCodec::OpusError::BadArg:
        return "OpusError::BadArg"sv;
    case USpeakNative::OpusCodec::OpusError::BufferToSmall:
        return "OpusError::BufferToSmall"sv;
    case USpeakNative::OpusCodec::OpusError::InternalError:
        return "OpusError::InternalError"sv;
    case USpeakNative::OpusCodec::OpusError::InvalidPacket:
        return "OpusError::InvalidPacket"sv;
    case USpeakNative::OpusCodec::OpusError::Unimplemented:
        return "OpusError::Unimplemented"sv;
    case USpeakNative::OpusCodec::OpusError::InvalidState:
        return "OpusError::InvalidState"sv;
    case USpeakNative::OpusCodec::OpusError::AllocFail:
        return "OpusError::AllocFail"sv;
    default:
        return "OpusError::Unkown"sv;
    }
}

}

#endif // USPEAK_OPUSERROR_H
