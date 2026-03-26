#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "ErrorCode.hpp"

namespace ullfh::core {

/**
 * Result of parsing a message from a packet buffer.
 *
 * Template parameter T is the parsed message type (e.g., AddOrderMessage).
 * Provides error status, parsed data pointer, and consumed byte count.
 *
 */
template <typename T>
class ParseResult {
   public:
    static_assert(std::is_trivially_copyable_v<T>,
                  "ParseResult<T> requires T to be trivially copyable");

    /**
     * Successful parse.
     */
    constexpr ParseResult(T* parsed_data, size_t bytes_consumed) noexcept
        : status_(ErrorCode::SUCCESS), data_(parsed_data), bytes_consumed_(bytes_consumed) {}

    /**
     * Parse failure.
     */
    constexpr explicit ParseResult(ErrorCode error) noexcept
        : status_(error), data_(nullptr), bytes_consumed_(0) {}

    // Default constructor: no error state.
    constexpr ParseResult() noexcept
        : status_(ErrorCode::SUCCESS), data_(nullptr), bytes_consumed_(0) {}

    inline constexpr ErrorCode status() const noexcept { return status_; }
    inline constexpr bool ok() const noexcept {
        return status_ == ErrorCode::SUCCESS;
    }

    inline constexpr T* data() const noexcept { return data_; }
    inline constexpr size_t bytes_consumed() const noexcept { return bytes_consumed_; }

    // Implicit conversion for boolean context (if ok) - hot-path, inlined
    inline constexpr explicit operator bool() const noexcept { return ok(); }

   private:
    ErrorCode status_;
    T* data_;
    size_t bytes_consumed_;
};

// Compile-time verification
static_assert(
    std::is_trivially_copyable_v<ParseResult<uint32_t>>,
    "ParseResult must be trivially copyable");

}  // namespace ullfh::core
