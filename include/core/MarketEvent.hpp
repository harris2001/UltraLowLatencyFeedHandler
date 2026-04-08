#pragma once

#include <cstdint>
#include <type_traits>

#include "Timestamp.hpp"

namespace ullfh::core {

/**
 * A MarketEvent is a normalized representation of a market data event, derived from raw ITCH messages.
 * It decouples exchange-specific message formats and is based on NASDAQ ITCH 5.0 TotalView protocol.
 * https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf
 * 
 * Each message is a fixed-size (56 bytes), trivially copyable, cache-line efficient structure that fits in L1 cache
 * 
 */
struct MarketEvent {
    /**
     * Event type enumeration matching ITCH message types.
     */
    enum class Type : uint8_t {
        // System events
        SYSTEM_EVENT = 0x00,                // 'S'
        STOCK_DIRECTORY = 0x01,             // 'R'
        STOCK_TRADING_ACTION = 0x02,        // 'H'
        REG_SHO_SHORT_SALE = 0x03,          // 'Y'
        MARKET_PARTICIPANT_POSITION = 0x04, // 'L'
        CIRCUIT_BREAKER_DECLINE = 0x05,     // 'V'
        CIRCUIT_BREAKER_STATUS = 0x06,      // 'W'
        IPO_UPDATE = 0x07,                  // 'K'
        LULD_UPDATE = 0x08,                 // 'J'
        OPERATIONS_HALT = 0x09,             // 'h'

        // Order book events
        ADD_ORDER = 0x0A,                   // 'A'
        ADD_ORDER_WITH_MPID = 0x0B,         // 'F'
        ORDER_EXECUTED = 0x0C,              // 'E'
        ORDER_EXECUTED_WITH_PRICE = 0x0D,   // 'C'
        ORDER_CANCEL = 0x0E,                // 'X'
        ORDER_DELETE = 0x0F,                // 'D'
        ORDER_REPLACE = 0x10,               // 'U'

        // Trade events
        TRADE = 0x11,                       // 'P'
        CROSS_TRADE = 0x12,                 // 'Q'
        BROKEN_TRADE = 0x13,                // 'B'

        // Net Order Imbalance Indicator
        NOII = 0x14,                        // 'I'

        // Retail order interest
        RETAIL_INTEREST = 0x15,             // 'N'

        // Direct listing
        DIRECT_LISTING = 0x16,              // 'O'
    };

    Type type;

    uint16_t stock_locate;
    uint16_t tracking_number;

    Timestamp exch_ts; // Exchange timestamp
    Timestamp recv_ts; // Ingress timestamp when we received the message (for latency measurement)

    // ---- Order/Trade Identity ----

    /**
     * Unique identifier for an order within a session.
     */
    uint64_t order_ref;

    // ---- Trade/Order Book Data ----

    /**
     * Quantity in shares.
     */
    uint32_t quantity;

    /**
     * Price in fixed-point: cents × 10,000 (e.g. 12345 = $1.2345)
     */
    int64_t price_int;

    /**
     * Side: 'B' (buy) or 'S' (sell).
     */
    char side;
    uint8_t flags;  // Bit 0=is_trade, 1=is_exec, 2=is_cancel, 3=is_printable
    uint8_t _pad[14]; // Pad to exactly one cache line (64 bytes)

    // ---- Constructors ----

    constexpr MarketEvent() noexcept
        : type(Type::SYSTEM_EVENT),
          stock_locate(0),
          tracking_number(0),
          exch_ts(Timestamp()),
          recv_ts(Timestamp()),
          order_ref(0),
          quantity(0),
          price_int(0),
          side('B'),
          flags(0) {}

    // ---- Helpers ----

    inline constexpr bool is_trade() const noexcept { return (flags & 0x01) != 0; }
    inline constexpr bool is_execution() const noexcept { return (flags & 0x02) != 0; }
    inline constexpr bool is_cancel() const noexcept { return (flags & 0x04) != 0; }
    inline constexpr bool is_printable() const noexcept { return (flags & 0x08) != 0; }

    inline constexpr void set_trade() noexcept { flags |= 0x01; }
    inline constexpr void set_execution() noexcept { flags |= 0x02; }
    inline constexpr void set_cancel() noexcept { flags |= 0x04; }
    inline constexpr void set_printable() noexcept { flags |= 0x08; }

    /**
     * Latency from ingress to event time (nanoseconds).
     * May be negative if clock skew occurs.
     * Hot-path: called frequently for latency measurement and histogramming.
     */
    inline constexpr int64_t latency_ns() const noexcept {
        return static_cast<int64_t>(recv_ts.count() - exch_ts.count());
    }
};

// ---- Compile-Time Assertions ----

/**
 * Verify trivial copyability: essential for zero-copy passing through SPSC queue.
 */
static_assert(std::is_trivially_copyable_v<MarketEvent>,
              "MarketEvent must be trivially copyable");

/**
 * Verify size is exactly one cache line (64 bytes): each event maps to one
 * cache line, eliminating false sharing when stored in the SPSC queue.
 */
static_assert(sizeof(MarketEvent) == 64,
              "MarketEvent must be exactly one cache line (64 bytes)");

/**
 * Verify standard layout for binary compatibility.
 */
static_assert(std::is_standard_layout_v<MarketEvent>,
              "MarketEvent must be standard layout for binary compatibility");

}  // namespace ullfh::core