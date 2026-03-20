#pragma once

#include <cstdint>
#include <string_view>

namespace ullfh::core {

/**
 * Error codes for parsing and processing messages.
 */

// clang-format off
#define ULLFH_ERROR_CODES(X) \
    X(SUCCESS,              0, "OK") \
    X(DECODE_ERROR,         1, "Decode error") \
    X(BOUNDS_CHECK_FAILED,  2, "Bounds check failed") \
    X(INVALID_MESSAGE_TYPE, 3, "Invalid message type") \
    X(QUEUE_FULL,           4, "Queue full") \
    X(BUFFER_EXHAUSTED,     5, "Buffer pool exhausted") \
    X(INVALID_TIMESTAMP,    6, "Invalid timestamp") \
    X(INVALID_PRICE,        7, "Invalid price")
// clang-format on

enum class ErrorCode : uint8_t {
#define X(name, code, msg) name = code,
    ULLFH_ERROR_CODES(X)
#undef X
};

inline std::string_view error_string(ErrorCode code) noexcept {
    switch (code) {
#define X(name, code, msg) \
    case ErrorCode::name:  \
        return msg;
        ULLFH_ERROR_CODES(X)
#undef X
        default:
            return "Unknown error";
    }
}

}  // namespace ullfh::core
