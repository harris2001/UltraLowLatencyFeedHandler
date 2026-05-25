#pragma once

#include <time.h>

#include <cstdint>
#include <type_traits>

namespace ullfh::core {

/**
 * This class is a high-resolution timestamp wrapper around a 64-bit nanosecond count.
 *
 * To better measure latency in diagnostics builds, we use CLOCK_MONOTONIC to capture time with nanosecond precision.
 * In release builds, the now() function is a no-op that returns 0 to eliminate syscall overhead from the hot path.
 */
class Timestamp {
   public:
    using Rep = uint64_t;

    constexpr Timestamp() noexcept : ns_(0) {}
    constexpr explicit Timestamp(Rep nanoseconds) noexcept : ns_(nanoseconds) {}

    static Timestamp now() noexcept {
#ifdef ULLFH_DIAGNOSTICS
        struct timespec ts;
        // CLOCK_REALTIME is required here: ITCH exchange timestamps are nanoseconds
        // since midnight (UTC wall-clock), so we must compare against wall time.
        // For kernel-bypass / hardware RX timestamping (SO_TIMESTAMPING), replace
        // this with the NIC-provided hardware timestamp from the socket ancillary data.
        clock_gettime(CLOCK_REALTIME, &ts);
        // We need to multiply seconds by 1 billion to convert to nanoseconds, then add the nanosecond part.
        return Timestamp(static_cast<Rep>(ts.tv_sec) * 1'000'000'000UL + ts.tv_nsec);
#else
        return Timestamp(0);
#endif
    }

    /**
     * Construct from raw ITCH nanosecond timestamp.
     */
    static constexpr Timestamp from_itch_ns(Rep nanoseconds) noexcept { return Timestamp(nanoseconds); }

    constexpr Rep count() const noexcept { return ns_; }

    /**
     * This overload allows us to measure the latency between two timestamps with zero overhead,
     * since we define it as inlined and rely on the compiler for optimization.
     */
    inline constexpr Rep operator-(const Timestamp& other) const noexcept { return ns_ - other.ns_; }

    inline constexpr bool operator<(const Timestamp& other) const noexcept { return ns_ < other.ns_; }

    inline constexpr bool operator==(const Timestamp& other) const noexcept { return ns_ == other.ns_; }

   private:
    Rep ns_;
};

// Compile-time assertion: must be trivially copyable to allow zero-copy passing through our SPSC queue.
static_assert(std::is_trivially_copyable_v<Timestamp>, "Timestamp must be trivially copyable");

}  // namespace ullfh::core
