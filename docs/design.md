# Design Decisions

This document captures the reasoning behind key architectural and implementation choices. For architecture diagrams, protocol reference, file layout, and build instructions see the [README](../README.md).

---

## No heap allocation on the hot path

Every allocation that could occur per-message (buffers, structs, temporaries) is either stack-allocated or carved out of pre-allocated member arrays. `MarketEvent`, `PacketBuffer`, and `GapRecord` are all trivially copyable POD structs. `SPSCQueue` stores items inline in a fixed member array. `MoldUDP64Receiver` pre-allocates its `mmsghdr`, `iovec`, and payload buffer arrays as class members, wired once at construction. This eliminates `malloc`/`free` from the critical path entirely.

## `recvmmsg` over `recvfrom`

`recvfrom` issues one syscall per datagram. At Nasdaq multicast rates, bursts of dozens of packets arrive within a single scheduling quantum. `recvmmsg` drains up to 32 datagrams per syscall, amortising the syscall overhead (~200–400 ns on a modern kernel) across the batch. The tradeoff is a small increase in per-message latency within a burst, which is acceptable - the alternative (missing packets due to kernel buffer overflow) is far worse.

## `ISocket` abstraction for kernel-bypass readiness

The MoldUDP64 and ITCH layers never touch a socket fd. All I/O is behind `ISocket::recv_batch()`. This means a DPDK, Solarflare OpenOnload, or Exasock implementation can be substituted by providing a new `ISocket` subclass with no changes to the protocol or pipeline code. The POSIX `UdpReceiver` is the first implementation; kernel-bypass is the anticipated next step when hardware is available.

## `SO_BUSY_POLL` instead of `epoll`

`epoll` requires a context switch into the kernel and back on each wakeup - typically 1–5 µs of overhead. `SO_BUSY_POLL` instructs the kernel to spin on the NIC receive queue for up to N microseconds before sleeping, keeping the data path in kernel context and avoiding the wakeup latency. The cost is one dedicated CPU core permanently spinning. For a feed handler where that core would otherwise be idle waiting for `epoll`, this is the correct trade.

## `SPSCQueue` with power-of-two capacity and separate cache lines

Modulo is replaced by a bitmask (`index & (N-1)`), turning what would be a division into a single AND instruction. Producer `head_` and consumer `tail_` are on separate `alignas(64)` cache lines so a write by the producer never invalidates the consumer's cache line and vice versa (false sharing would serialize the two threads through the cache coherence protocol). Acquire-release ordering (`memory_order_release` on push, `memory_order_acquire` on pop) provides the minimum necessary synchronisation without a full memory barrier.

## `MarketEvent` sized to exactly one cache line

All 23 ITCH 5.0 message types are normalised into a single 64-byte struct. This means every slot in the SPSC ring is exactly one cache line - a `try_push` writes one cache line and a `try_pop` reads one cache line, with no straddling. The fixed-size layout also eliminates any per-message dynamic dispatch in the consumer.

## Fixed-point price arithmetic

ITCH prices are transmitted as integers scaled by 10,000 (4 decimal places). They are stored as `int64_t price_int` in `MarketEvent` without conversion to `double`. Floating-point conversion is deferred to logging paths only (`Price::to_double()`). This avoids FP pipeline stalls and rounding artefacts on the hot path.

## ITCH decode via `switch` jump table, not virtual dispatch

`ITCHDispatcher::dispatch()` is a single `switch(data[0])` over the message type byte. With `-O2` or higher, GCC and Clang compile a switch over a dense set of chars into a jump table - one indirect branch, no function call overhead. All `decode_*` helpers are `[[gnu::always_inline]]` so the compiler inlines them directly into the jump table targets. An alternative design using a `std::array<decode_fn*, 256>` function-pointer table was considered but abandoned because it requires a global initialisation step and introduces a level of indirection that hinders inlining.

## `Timestamp::now()` is a no-op in Release

Capturing `clock_gettime(CLOCK_REALTIME)` on every received message costs ~20–50 ns per call. In Release builds (`-DNDEBUG`, no `ULLFH_DIAGNOSTICS`), `Timestamp::now()` returns a zero timestamp immediately. Latency measurement is enabled only under `ULLFH_DIAGNOSTICS` (Debug builds). This means the timestamp capture can be left in the hot-path code without paying any cost in production.

## Gap detection without re-request

Issuing a MoldUDP64 re-request packet from the receive thread would introduce RTT latency (typically 50–500 µs to a re-request server) and require a second socket. Both would stall or complicate the hot path. Instead, gaps are reported as `GapRecord` values forwarded to `IParser::on_gap()` and the receiver continues processing from the received sequence number immediately. The application layer determines recovery strategy (log, alert, snapshot-restart, or re-request on a separate thread).

## `SCHED_FIFO` and core pinning

The OS scheduler can preempt a thread mid-packet and not resume it for milliseconds. `SCHED_FIFO` with a high priority prevents preemption by lower-priority threads. `pthread_setaffinity_np` binds the receive thread to a dedicated core so it never migrates, keeping L1/L2 cache state warm. Both calls are made once at thread startup and never repeated - they have zero hot-path cost.

## `spdlog` macros compile to nothing in Release

`ULLFH_LOG_TRACE`, `ULLFH_LOG_DEBUG`, `ULLFH_LOG_INFO`, and `ULLFH_LOG_WARN` expand to `do{}while(0)` when `ULLFH_DIAGNOSTICS` is not defined. Only `ULLFH_LOG_ERROR` and `ULLFH_LOG_CRITICAL` are always active. This means diagnostic logging calls can be placed freely in the hot path without any Release-build overhead.

## `ParseResult<T>` over exceptions

Exceptions have non-zero cost even when not thrown (larger binaries, inhibited inlining, stack unwinding tables). `ParseResult<T>` is a trivially copyable result type carrying either a pointer to the parsed data + bytes consumed, or an `ErrorCode`. The `explicit operator bool` enables clean if-guard idiom. No exception handling machinery is pulled into the hot path.
