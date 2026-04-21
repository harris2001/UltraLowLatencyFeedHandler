# Ultra-Low Latency Market Data Feed Handler

A high-performance NASDAQ TotalView-ITCH 5.0 feed handler written in modern C++20. Receives UDP multicast data via MoldUDP64, decodes all 23 ITCH message types, and delivers normalised `MarketEvent` structs to a consumer through a lock-free SPSC queue — with no heap allocation on the hot path.

> **Design decisions** (why each choice was made) are documented in [docs/design.md](docs/design.md).

---

## Architecture

```C++
NIC (UDP multicast)
        │
        ▼
 ┌──────────────────────────────────────────────────────┐
 │     Receiver Thread (pinned core, SCHED_FIFO)        │
 |──────────────────────────────────────────────────────|
 │  UdpReceiver::recv_batch() <- recvmmsg, batch=32     │
 │        │                                             │
 │        ▼                                             │
 │  MoldUDP64Receiver::process_packet()                 │
 │   ├─ Session / sequence-number validation            │
 │   ├─ Gap detection -> GapRecord -> IParser::on_gap() │
 │   └─ Message block iteration                         │
 │        │                                             │
 │        ▼                                             │
 │  ITCHDispatcher::dispatch()                          │
 │   └─ switch(msg_type) -> decode_* -> MarketEvent     │
 │        │                                             │
 │        ▼                                             │
 │  SPSCQueue<MarketEvent, 65536>::try_push()           │
 └──────────────────────────────────────────────────────┘
        │  (cache-line store + release fence)
        ▼
 ┌──────────────────────────────────────────────────────┐
 │            Consumer Thread  (pinned core)            │
 |──────────────────────────────────────────────────────|
 │  SPSCQueue::try_pop() -> IParser::on_market_event()  │
 │   └─ Strategy / logger / normaliser / forwarder      │
 └──────────────────────────────────────────────────────┘
```

### Pipeline layers

| Layer | Component | Responsibility |
|---|---|---|
| Transport | `UdpReceiver` | POSIX UDP multicast socket; `recvmmsg` batch receive |
| Framing | `MoldUDP64Receiver` | Strip MoldUDP64 header; validate session & sequence; gap detection |
| Decoding | `ITCHDispatcher` | Zero-copy ITCH 5.0 decode -> `MarketEvent` |
| Transfer | `SPSCQueue` | Lock-free ring buffer; one cache line per slot |
| Consumption | `IParser` | Application-defined handler (strategy, logger, forwarder) |
| Threading | `Pipeline` | Thread ownership, core pinning, SCHED_FIFO |

---

## Protocol Support

### MoldUDP64 (framing)

Every Nasdaq UDP datagram is a MoldUDP64 downstream packet:

```
Offset  Field            Size
──────────────────────────────────────
 0      Session          10   space-padded ASCII
10      Sequence Number   8   uint64, big-endian
18      Message Count     2   uint16, big-endian  (0x0000=heartbeat, 0xFFFF=end-of-session)
20      Message Block 0   …
        Message Block N   …
```

Each message block: 2-byte big-endian length followed by the ITCH payload.

### ITCH 5.0 message types

| Char | Type | Char | Type |
|---|---|---|---|
| `S` | System Event | `A` | Add Order |
| `R` | Stock Directory | `F` | Add Order w/ MPID |
| `H` | Stock Trading Action | `E` | Order Executed |
| `Y` | Reg SHO | `C` | Order Executed w/ Price |
| `L` | Market Participant Position | `X` | Order Cancel |
| `V` | MWCB Decline Level | `D` | Order Delete |
| `W` | MWCB Status | `U` | Order Replace |
| `K` | IPO Quoting Period Update | `P` | Trade (Non-Cross) |
| `J` | LULD Auction Collar | `Q` | Cross Trade |
| `h` | Operational Halt | `B` | Broken Trade |
| `I` | NOII | `N` | RPII |
| `O` | DLCR Price Discovery | | |

---

## Key Data Structures

### `MarketEvent` — normalised 64-byte message

All ITCH message types are decoded into a single cache-line-sized struct:

```
Field            Offset  Size  Notes
────────────────────────────────────────────────────
type              0       1    MarketEvent::Type enum
stock_locate      1       2
tracking_number   3       2
(pad)             5       3
exch_ts           8       8    exchange nanosecond timestamp
recv_ts          16       8    ingress timestamp
order_ref        24       8    order reference or match number
quantity         32       4    shares
(pad)            36       4
price_int        40       8    fixed-point × 10,000
side             48       1    'B' or 'S'
flags            49       1    bit 0=trade 1=exec 2=cancel 3=printable
(pad)            50      14
Total            64
```

### `SPSCQueue<T, N>` — lock-free ring buffer

Power-of-two capacity, `alignas(64)` producer/consumer indices on separate cache lines. `try_push` / `try_pop` are wait-free. No heap allocation.

### `GapRecord` — sequence gap descriptor

Carries session (10 bytes), `expected_seq`, `received_seq`, and ingress timestamp. Forwarded to `IParser::on_gap()` — the application decides recovery strategy.

### `ISocket` — transport abstraction

Pure virtual interface (`open`, `close`, `recv_batch`). `UdpReceiver` is the POSIX implementation. A DPDK or OpenOnload backend can be substituted without touching the protocol or pipeline layers.

---

## Socket Tuning

| Option | Value | Purpose |
|---|---|---|
| `SO_RCVBUF` | 8 MB | Absorb microsecond bursts |
| `SO_BUSY_POLL` | 50 µs | Kernel-side busy-wait, avoids epoll wakeup latency |
| `SO_REUSEADDR` | on | Allow fast restart |
| `IP_ADD_MEMBERSHIP` | — | ASM multicast join |
| `IP_ADD_SOURCE_MEMBERSHIP` | — | SSM multicast join |
| `SOCK_NONBLOCK` | — | Non-blocking; pairs with busy-poll loop |

---

## Latency Targets (x86-64, Release)

| Stage | Target |
|---|---|
| `recvmmsg` batch of 32 packets end-to-end | < 2 µs |
| Single MoldUDP64 packet (10 Add Orders) | < 500 ns |
| `ITCHDispatcher::dispatch` per message | < 30 ns |
| `SPSCQueue::try_push` / `try_pop` | < 10 ns |
| NIC ingress -> `IParser::on_market_event` | < 5 µs |

<!-- ---

## File Layout

```
include/
├── core/
│   ├── ErrorCode.hpp          Error codes (X-macro, O(1) lookup)
│   ├── GapRecord.hpp          Sequence-gap descriptor
│   ├── Logger.hpp             spdlog singleton (zero-cost macros in Release)
│   ├── MarketEvent.hpp        Normalised 64-byte market event
│   ├── PacketBuffer.hpp       Non-owning UDP packet descriptor
│   ├── ParseResult.hpp        Generic result<T, ErrorCode>
│   └── Timestamp.hpp          64-bit nanosecond clock (no-op in Release)
├── networking/
│   ├── Receiver.hpp           ISocket abstract interface
│   └── SocketConfig.hpp       Socket configuration
├── network/
│   └── UdpReceiver.hpp        POSIX ISocket implementation
├── pipeline/
│   └── SPSCQueue.hpp          Lock-free SPSC ring buffer
└── protocols/
    ├── IParser.hpp             Consumer callbacks
    ├── itch/
    │   ├── ITCHMessages.hpp    23 ITCH 5.0 wire structs + BE helpers
    │   └── ITCHDispatcher.hpp  Zero-allocation decode -> MarketEvent
    └── mold/
        ├── MoldUDP64Header.hpp     Wire types for downstream packet + message block
        └── MoldUDP64Receiver.hpp   Template receiver (ISocket -> SPSCQueue)

src/
├── main.cpp
├── app/
│   ├── Pipeline.cpp           Thread ownership and lifecycle
│   └── ThreadPinning.cpp      pthread_setaffinity_np, SCHED_FIFO
├── network/
│   └── UdpReceiver.cpp
└── protocols/itch/
    └── Decoder.cpp            ITCHDispatcher implementation

tests/
└── core/types_test.cpp        GoogleTest unit tests

benchmarks/
└── main.cpp                   Google Benchmark stubs
``` -->

---

## Requirements

- Linux (only tested platform)
- C++20 compiler — GCC ≥ 13 or Clang ≥ 16
- CMake ≥ 3.28

## Dependencies

All fetched automatically via CMake `FetchContent` — no system installation required.

| Library | Version | Purpose |
|---|---|---|
| [spdlog](https://github.com/gabime/spdlog) | v1.15.3 | Structured logging |
| [GoogleTest](https://github.com/google/googletest) | v1.14.0 | Unit tests |
| [Google Benchmark](https://github.com/google/benchmark) | v1.8.3 | Microbenchmarks |

## Build

```bash
git clone https://github.com/harris2001/UltraLowLatencyFeedHandler.git
cd UltraLowLatencyFeedHandler

# Release
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug (AddressSanitizer + diagnostics)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

## Benchmarks

```bash
./build/benchmarks/ullfh_benchmarks
```
