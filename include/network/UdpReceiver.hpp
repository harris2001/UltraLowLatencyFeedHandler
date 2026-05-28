#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include "network/Receiver.hpp"
#include "network/SocketConfig.hpp"

namespace ullfh::network {

/**
 * POSIX UDP multicast receiver implementing ISocket.
 *
 * Uses recvmmsg(2) to drain up to k_batch_size datagrams per syscall.
 *
 * To swap in a kernel-bypass backend, implement ISocket with DPDK/OpenOnload
 * and pass that implementation to MoldUDP64Receiver instead.
 */
class UdpReceiver : public ISocket {
   public:
    // Inherits k_batch_size = 32 and k_max_payload = 1500 from ISocket.

    UdpReceiver() noexcept;
    ~UdpReceiver() noexcept override;

    UdpReceiver(const UdpReceiver&) = delete;
    UdpReceiver& operator=(const UdpReceiver&) = delete;
    UdpReceiver(UdpReceiver&&) = delete;
    UdpReceiver& operator=(UdpReceiver&&) = delete;

    bool open(const SocketConfig& cfg) noexcept override;

    void close() noexcept override;

    int recv_batch(mmsghdr* msgs, int max_batch) noexcept override;

};

}  // namespace ullfh::network
