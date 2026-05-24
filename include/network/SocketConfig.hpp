#pragma once

#include <cstdint>
#include <string_view>

namespace ullfh::network {

/**
 * Configuration settings for the UDP multicast receving socket.
 * This is passed to the ISocket::open() at startup.
 */
struct SocketConfig {
    // Network interface to bind to (e.g. "eth0", "bond0") or default 0.0.0.0 if left empty
    std::string_view interface_name{};

    // IPv4 doted-decimal (e.g. "233.54.12.111")
    std::string_view multicast_group{};

    // Source-specific multicast (SSM) address or ASM (any-source multicast) if empty
    std::string_view source_ip{};

    // UDP listening port
    uint16_t port{0};

    // SO_RCVBUF (socket buffer size in bytes - default 8MB which is reasonable for Nasdaq multicast rates)
    int rcvbuf_bytes{8 * 1024 * 1024};

    // SO_BUSY_POLL busy wait on the NIC receive queue in microseconds (0 => disabled)
    int busy_poll_us{50};

    // SO_TIMESTAMPING hardware timestamps but needs NIC & driver support (off by default)
    bool hw_timestamps{false};
};

}  // namespace ullfh::network
