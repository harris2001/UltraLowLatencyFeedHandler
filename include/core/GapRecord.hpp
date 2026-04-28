#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

#include "Timestamp.hpp"

namespace ullfh::core {

/**
 * Since MoldUDP64 can drop packages, we need a mechanism to detect them on the receiver and forward them to 
 * IParser::on_gap().The application layer can then decide whether to request retransmission or absorb the loss.
 */
struct GapRecord {
    // Session identifier is always 10 ASCII bytes, space-padded
    char session[10];

    uint8_t _pad0[6]; // explicit pad to 8-byte-align the sequence fields

    uint64_t expected_seq;
    uint64_t received_seq;
    Timestamp detected_ts;

    // Return the number of messages missing
    [[nodiscard]] constexpr uint64_t gap_size() const noexcept {
        return received_seq - expected_seq;
    }

    // Return session as a string_view
    [[nodiscard]] inline std::string_view session_view() const noexcept {
        return std::string_view(session, 10);
    }
};

static_assert(std::is_trivially_copyable_v<GapRecord>,
              "GapRecord must be trivially copyable for lock-free transfer");
static_assert(sizeof(GapRecord) == 40,
              "GapRecord size unexpected");

} // namespace ullfh::core
