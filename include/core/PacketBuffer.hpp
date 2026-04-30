#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "Timestamp.hpp"

namespace ullfh::core {

/**
 * Lightweight descriptor for a received UDP packet.
 *
 * Holds buffer metadata: payload span, ingress timestamp, and source identifier.
 * The actual data is managed by a pool; this struct only carries references.
 *
 */
struct PacketBuffer {
    uint8_t* data; // Packet payload ptr owned by pool
    size_t capacity;
    size_t length;
    Timestamp recv_ts; //ingress time (captured in the async receive callback)
    uint32_t source_id; // multicast channel | feed source | virtual system ID

    constexpr PacketBuffer() noexcept
        : data(nullptr), capacity(0), length(0), recv_ts(Timestamp()), source_id(0) {}

    /**
     * Construct a descriptor for a preallocated buffer.
     *
     * @param buf         pointer to buffer memory
     * @param cap         total capacity in bytes
     * @param ts          ingress timestamp
     * @param src_id      source/channel identifier
     */
    constexpr PacketBuffer(uint8_t* buf, size_t cap, Timestamp ts, uint32_t src_id) noexcept
        : data(buf), capacity(cap), length(0), recv_ts(ts), source_id(src_id) {}

    /**
     * Return the buffer as a byte span
     */
    inline constexpr std::span<uint8_t> span() const noexcept {
        return std::span<uint8_t>(data, length);
    }

    inline constexpr size_t available() const noexcept { return capacity - length; }

    /**
     * Reset for reuse: clear length, keep capacity and data pointer (mostly fro pool recycling).
     */
    inline constexpr void reset() noexcept { length = 0; }
};

static_assert(std::is_trivially_copyable_v<PacketBuffer>,
              "PacketBuffer must be trivially copyable");
static_assert(sizeof(PacketBuffer) <= 64,
              "PacketBuffer should fit in one cache line for allocation efficiency");

}  // namespace ullfh::core
