#pragma once

namespace ullfh::app {

/**
 * @brief Thread affinity and scheduler helpers.
 *
 * @note All functions are called once at thread startup - NEVER on the hot path.
 * @note Failures are non-fatal: the system continues at normal scheduling priority while logging warnings.
 */

/**
 * @brief Uses pthread_setaffinity_np(3).
 *
 * @param core_id  Zero-based logical CPU core index.
 * @return         true on success; false with errno preserved on failure.
 */
bool pin_to_core(int core_id) noexcept;

/**
 * @brief Sets the calling thread's scheduling policy to `SCHED_FIFO` at the given real-time priority.
 *
 * @note Requires CAP_SYS_NICE or root. 
 * @note If the process lacks the capability this function logs a warning and returns false - it does NOT crash.
 *
 * @param priority  Real-time priority in [1, 99]. Higher = more urgent.
 *                  Typical choice for a receive thread: 80-90.
 * @return          true on success; false with errno preserved on failure.
 */
bool set_realtime_priority(int priority) noexcept;

/**
 * @brief Convenience wrapper
 *
 * Usage: 
 *    - pins thread to core
 *    - sets `SCHED_FIFO` priority.
 *
 * @note Logs a warning for each step that fails but always returns after both attempts.
 *
 * @param core_id   Core to pin to (negative = skip pinning).
 * @param priority  RT priority (0 = skip RT scheduling).
 */
void configure_thread(int core_id, int priority) noexcept;

}  // namespace ullfh::app
