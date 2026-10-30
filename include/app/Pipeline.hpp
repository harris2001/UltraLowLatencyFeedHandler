#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "ThreadPinning.hpp"
#include "../network/SocketConfig.hpp"
#include "../network/UdpReceiver.hpp"
#include "../pipeline/SPSCQueue.hpp"
#include "../protocols/IParser.hpp"
#include "../protocols/itch/ITCHDispatcher.hpp"
#include "../protocols/mold/MoldUDP64Header.hpp"
#include "../protocols/mold/MoldUDP64Receiver.hpp"

namespace ullfh::app {

/**
 * @brief Configuration for the ULLFH processing pipeline.
 *
 * Groups thread, socket, and scheduling configuration in one place so that
 * pipeline behaviour can be configured, maintained, and measured consistently.
 *
 * The configuration is also intended to provide a foundation for future
 * per-pipeline logging, metrics, and performance monitoring.
 */
struct PipelineConfig {

    /**
     * A dedicated UDP socket dedicated to receiving market data.
     */
    network::SocketConfig socket{};

    /**
     * @brief CPU core index for the receive/decode thread.
     *
     * Usage:
     *   - receives UDP packets by calling recvmmsg
     *   - unwraps MoldUDP64
     *   - dispatches ITCH messages
     *
     * @note Set to `-1` to disable pinning (default)
     * @note Use indexes `0` to `nproc - 1` to pin to that core
     */
    int recv_core{-1};

    /**
     * @brief CPU core index for the consumer thread.
     *
     * Usage:
     *   - calls `protocols::IParser` callbacks
     *
     * @note Set to `-1` to disable pinning (default)
     * @note Use indexes `0` to `nproc - 1` to pin to that core
     */
    int consumer_core{-1};

    /**
    * @brief Linux real-time scheduling priority for the pipeline threads.
    *
    * Setup:
    *    - 0 = Use normal SCHED_OTHER scheduling (default)
    *    - 1...99 = Request SCHED_FIFO with the specified real-time priority (higher value => higher priority)
    *
    * @note:
    *   To modify this value make sure you have CAP_SYS_NICE or running it as root
    *
    * @warning:
    *   Choosing a very high scheduling priority can cause CPU starvation for other Linux processes 
    *   Use this as a tuning parameter (i.e. don't just set it to 99 and pray your system doesn't crash :))))
    */
    int rt_priority{0};
};

/**
 * @brief Full feed handler pipeline.
 *
 * Owns the UDP socket, the SPSC queue, the MoldUDP64 receiver, and the two threads (recv/decode and consumer).
 * The caller provides an IParser implementation to receive decoded MarketEvents.
 *
 * Usage:
 * @code
 * //Lifecycle:
 *
 *   Pipeline p(cfg, &my_parser);
 *   if (!p.start()) 
 *   {
 *      //handle error 
 *   }
 *   // ... feed is running ...
 *   p.stop();   // blocks until both threads exit
 * @endcode
 *
 * @warning 
 *    Thread safety: 
 *    `start()` / `stop()` must only be called from the owner thread.
 */
class Pipeline {
   public:
    /** SPSC ring capacity: 65'536 x 64 B = 4 MB. */
    static constexpr std::size_t k_queue_capacity = 65'536;

    /**
     * @param cfg     Pipeline configuration.
     * @param parser  Consumer callbacks (event, gap, session end).
     *                
     * @warning The provided parser must outlive the Pipeline.
     */
    Pipeline(const PipelineConfig& cfg, protocols::IParser* parser) noexcept;
    ~Pipeline() noexcept;

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    /**
     * @brief Open the socket, start the receive thread and consumer thread.
     *
     * @return 
     *    - `true` when both threads started
     *    - `false` when socket failed to open.
     */
    [[nodiscard]] bool start() noexcept;

    /**
     * @brief Signal both threads to stop and join them. Blocks until clean exit.
     */
    void stop() noexcept;

   private:
    using MoldReceiver = protocols::mold::MoldUDP64Receiver<k_queue_capacity>;
    using Queue = pipeline::SPSCQueue<core::MarketEvent, k_queue_capacity>;

    PipelineConfig cfg_;
    protocols::IParser* parser_;

    // Components owned (in the correct order for construction / destruction)
    network::UdpReceiver socket_;
    protocols::itch::ITCHDispatcher dispatcher_;
    Queue queue_;
    std::unique_ptr<MoldReceiver> mold_receiver_;

    // Threads
    std::thread recv_thread_;
    std::thread consumer_thread_;

    // Thread entry points
    void recv_loop() noexcept;
    void consumer_loop() noexcept;
};

}  // namespace ullfh::app
