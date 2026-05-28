#pragma once

#include <sys/socket.h>

#include "network/SocketConfig.hpp"

namespace ullfh::network {

/**
 * Abstract socket interface
 *
 * It decouples the MoldUDP64 receiver from the underlying transport to allow 
 * for easier substitution of the Socket. Config with kernel bypass methods 
 * such as DPDK without needing to modify the protocol or the pipeline layers.
 */
class ISocket {
   public:
    /** Maximum datagrams drained per recvmmsg() call. */
    static constexpr int k_batch_size = 32;
    /** Maximum UDP payload accepted (standard Ethernet MTU). */
    static constexpr int k_max_payload = 1500;

    ISocket() = default;
    ISocket(const ISocket&) = delete;
    ISocket& operator=(const ISocket&) = delete;
    ISocket(ISocket&&) = delete;
    ISocket& operator=(ISocket&&) = delete;
    virtual ~ISocket() = default;

    /**
     * Opens the socket and join the multicast group
     *
     * @param cfg: Socket Configuration
     * @return true: success
     *         false: system error
     */
    virtual bool open(const SocketConfig& cfg) noexcept = 0;

    /**
     * Leave the multicast group and close the socket.
     * Safe to call even if open() was never called or failed.
     */
    virtual void close() noexcept = 0;

    /**
     * Receive up to max_batch datagrams in a single syscall via recvmmsg(2).
     *
     * @param msgs       array of message headers.
     * @param max_batch  size of the msgs array.
     * @return           number of datagrams received (>=0), or -1 on error.
     *
     * Note: The caller is responsible for populating 
     *       a) msgs[].msg_hdr.msg_iov and
     *       b) msg_iovlen before calling. 
     *       On return, msgs[i].msg_len contains the number of bytes received in each datagram.
     */
    virtual int recv_batch(mmsghdr* msgs, int max_batch) noexcept = 0;
};

}  // namespace ullfh::network
