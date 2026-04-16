#include <gtest/gtest.h>

#include <type_traits>

#include "core/ErrorCode.hpp"
#include "core/MarketEvent.hpp"
#include "core/PacketBuffer.hpp"
#include "core/ParseResult.hpp"
#include "core/Timestamp.hpp"
#include "protocols/itch/ITCHMessages.hpp"
#include "protocols/itch/ITCHDecoder.hpp"

namespace ullfh::core::test {

// ============================================================================
// Timestamp Tests
// ============================================================================

TEST(TimestampTest, IsTriviallyCopyable) {
    EXPECT_TRUE(std::is_trivially_copyable_v<Timestamp>);
}

TEST(TimestampTest, DefaultConstruction) {
    Timestamp ts;
    EXPECT_EQ(ts.count(), 0);
}

TEST(TimestampTest, ExplicitConstruction) {
    Timestamp ts(12345);
    EXPECT_EQ(ts.count(), 12345);
}

TEST(TimestampTest, FromITCH) {
    Timestamp ts = Timestamp::from_itch_ns(999999);
    EXPECT_EQ(ts.count(), 999999);
}

TEST(TimestampTest, Subtraction) {
    Timestamp ts1(1000);
    Timestamp ts2(500);
    EXPECT_EQ(ts1 - ts2, 500);
}

// ============================================================================
// ErrorCode Tests
// ============================================================================

TEST(ErrorCodeTest, EnumValuesValid) {
    EXPECT_EQ(static_cast<int>(ErrorCode::SUCCESS), 0);
    EXPECT_EQ(static_cast<int>(ErrorCode::DECODE_ERROR), 1);
    EXPECT_EQ(static_cast<int>(ErrorCode::QUEUE_FULL), 4);
}

TEST(ErrorCodeTest, ErrorStringValid) {
    EXPECT_EQ(error_string(ErrorCode::SUCCESS), "OK");
    EXPECT_EQ(error_string(ErrorCode::DECODE_ERROR), "Decode error");
}

// ============================================================================
// ParseResult Tests
// ============================================================================

TEST(ParseResultTest, IsTriviallyCopyable) {
    EXPECT_TRUE((std::is_trivially_copyable_v<ParseResult<uint32_t>>));
}

TEST(ParseResultTest, SuccessConstruction) {
    uint32_t data = 42;
    ParseResult<uint32_t> result(&data, 10);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.status(), ErrorCode::SUCCESS);
    EXPECT_EQ(result.bytes_consumed(), 10);
    EXPECT_EQ(*result.data(), 42);
}

TEST(ParseResultTest, ErrorConstruction) {
    ParseResult<uint32_t> result(ErrorCode::BOUNDS_CHECK_FAILED);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.status(), ErrorCode::BOUNDS_CHECK_FAILED);
}

// ============================================================================
// PacketBuffer Tests
// ============================================================================

TEST(PacketBufferTest, IsTriviallyCopyable) {
    EXPECT_TRUE(std::is_trivially_copyable_v<PacketBuffer>);
}

TEST(PacketBufferTest, SizeConstraint) {
    EXPECT_LE(sizeof(PacketBuffer), 64);
}

TEST(PacketBufferTest, DefaultConstruction) {
    PacketBuffer buf;
    EXPECT_EQ(buf.data, nullptr);
    EXPECT_EQ(buf.length, 0);
    EXPECT_EQ(buf.capacity, 0);
}

TEST(PacketBufferTest, ResetClearsLength) {
    uint8_t data[100];
    PacketBuffer buf(&data[0], 100, Timestamp::from_itch_ns(123), 1);
    buf.length = 50;
    buf.reset();
    EXPECT_EQ(buf.length, 0);
    EXPECT_EQ(buf.capacity, 100);
}

// ============================================================================
// MarketEvent Tests
// ============================================================================

TEST(MarketEventTest, IsTriviallyCopyable) {
    EXPECT_TRUE(std::is_trivially_copyable_v<core::MarketEvent>);
}

TEST(MarketEventTest, SizeIsEfficient) {
    // 64 bytes == exactly one cache line: each event maps 1-to-1 to a cache
    // line, eliminating false sharing in the SPSC queue.
    EXPECT_EQ(sizeof(core::MarketEvent), 64);
}

TEST(MarketEventTest, AlignmentIs8Bytes) {
    // Aligned to 8-byte boundary (uint64_t fields)
    // Sufficient for SIMD and cache efficiency
    EXPECT_EQ(alignof(core::MarketEvent), 8);
}

TEST(MarketEventTest, IsStandardLayout) {
    EXPECT_TRUE(std::is_standard_layout_v<core::MarketEvent>);
}

TEST(MarketEventTest, DefaultConstruction) {
    core::MarketEvent evt;
    EXPECT_EQ(evt.type, core::MarketEvent::Type::SYSTEM_EVENT);
    EXPECT_EQ(evt.quantity, 0);
    EXPECT_EQ(evt.price_int, 0);
    EXPECT_EQ(evt.side, 'B');
}

TEST(MarketEventTest, FlagsWork) {
    core::MarketEvent evt;
    EXPECT_FALSE(evt.is_trade());
    EXPECT_FALSE(evt.is_execution());

    evt.set_trade();
    EXPECT_TRUE(evt.is_trade());
    EXPECT_FALSE(evt.is_execution());

    evt.set_execution();
    EXPECT_TRUE(evt.is_execution());
}

TEST(MarketEventTest, LatencyCalculation) {
    core::MarketEvent evt;
    // exch_ts is when the exchange sent the message, recv_ts is when we received it.
    // recv_ts > exch_ts in normal operation, so latency is positive.
    evt.exch_ts = Timestamp::from_itch_ns(500);
    evt.recv_ts = Timestamp::from_itch_ns(1000);
    EXPECT_EQ(evt.latency_ns(), 500);
}

// ============================================================================
// ITCH Message Structure Tests
// ============================================================================

TEST(ITCHMessagesTest, SystemEventMessageSize) {
    using Message = protocols::itch::SystemEventMessage;
    EXPECT_TRUE(std::is_trivially_copyable_v<Message>);
    EXPECT_EQ(sizeof(Message), 12);
}

TEST(ITCHMessagesTest, AddOrderMessageSize) {
    using Message = protocols::itch::AddOrderMessage;
    EXPECT_TRUE(std::is_trivially_copyable_v<Message>);
    EXPECT_EQ(sizeof(Message), 36);
}

TEST(ITCHMessagesTest, OrderExecutedMessageSize) {
    using Message = protocols::itch::OrderExecutedMessage;
    EXPECT_TRUE(std::is_trivially_copyable_v<Message>);
    EXPECT_EQ(sizeof(Message), 31);
}

TEST(ITCHMessagesTest, TradeMessageSize) {
    using Message = protocols::itch::TradeMessage;
    EXPECT_TRUE(std::is_trivially_copyable_v<Message>);
    EXPECT_EQ(sizeof(Message), 44);
}

// ============================================================================
// TradingActionReason / TradingResumptionReason decode tests
// ============================================================================

using namespace protocols::itch;

TEST(TradingReasonTest, HaltCodeDecodesAsHalt) {
    // T1 → HALT_NEWS_PENDING (first entry in reason_table)
    Reason r{'T', '1', ' ', ' '};
    auto result = decode_halt_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingActionReason::HALT_NEWS_PENDING);
}

TEST(TradingReasonTest, HaltCodeMWC3DecodesAsHalt) {
    // MWC3 → MARKET_WIDE_CIRCUIT_BREAKER_HALT_LEVEL_3
    Reason r{'M', 'W', 'C', '3'};
    auto result = decode_halt_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingActionReason::MARKET_WIDE_CIRCUIT_BREAKER_HALT_LEVEL_3);
}

TEST(TradingReasonTest, HaltCodeIPO1DecodesAsHalt) {
    // IPO1 → IPO_ISSUE_NOT_YET_TRADING
    Reason r{'I', 'P', 'O', '1'};
    auto result = decode_halt_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingActionReason::IPO_ISSUE_NOT_YET_TRADING);
}

TEST(TradingReasonTest, ResumptionCodeT3DecodesAsResumption) {
    // T3 → NEWS_AND_RESUMPTION_TIMES (first entry in resumption_reason_table)
    Reason r{'T', '3', ' ', ' '};
    auto result = decode_resumption_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingResumptionReason::NEWS_AND_RESUMPTION_TIMES);
}

TEST(TradingReasonTest, ResumptionCodeMWCQDecodesAsResumption) {
    // MWCQ → MARKET_WIDE_CIRCUIT_BREAKER_RESUMPTION
    Reason r{'M', 'W', 'C', 'Q'};
    auto result = decode_resumption_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingResumptionReason::MARKET_WIDE_CIRCUIT_BREAKER_RESUMPTION);
}

TEST(TradingReasonTest, ResumptionCodeIPOQDecodesAsResumption) {
    // IPOQ → IPO_SECURITY_RELEASED_FOR_QUOTATION
    Reason r{'I', 'P', 'O', 'Q'};
    auto result = decode_resumption_reason(r);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, TradingResumptionReason::IPO_SECURITY_RELEASED_FOR_QUOTATION);
}

// Cross-table exclusivity: a halt code must not appear in the resumption table and vice versa.

TEST(TradingReasonTest, HaltCodeReturnsNulloptFromResumptionDecoder) {
    // T1 is a halt code — resumption decoder must reject it.
    Reason r{'T', '1', ' ', ' '};
    EXPECT_FALSE(decode_resumption_reason(r).has_value());
}

TEST(TradingReasonTest, ResumptionCodeReturnsNulloptFromHaltDecoder) {
    // T3 is a resumption code — halt decoder must reject it.
    Reason r{'T', '3', ' ', ' '};
    EXPECT_FALSE(decode_halt_reason(r).has_value());
}

TEST(TradingReasonTest, MWCQIsExclusiveToResumptionTable) {
    // MWCQ (resumption) vs MWC1/2/3/0 (halt) — must not cross.
    Reason mwcq{'M', 'W', 'C', 'Q'};
    EXPECT_FALSE(decode_halt_reason(mwcq).has_value());
    EXPECT_TRUE(decode_resumption_reason(mwcq).has_value());

    Reason mwc1{'M', 'W', 'C', '1'};
    EXPECT_TRUE(decode_halt_reason(mwc1).has_value());
    EXPECT_FALSE(decode_resumption_reason(mwc1).has_value());
}

TEST(TradingReasonTest, IPO1VsIPOQVsIPOEAreExclusive) {
    Reason ipo1{'I', 'P', 'O', '1'};
    EXPECT_TRUE(decode_halt_reason(ipo1).has_value());
    EXPECT_FALSE(decode_resumption_reason(ipo1).has_value());

    Reason ipoq{'I', 'P', 'O', 'Q'};
    EXPECT_FALSE(decode_halt_reason(ipoq).has_value());
    EXPECT_TRUE(decode_resumption_reason(ipoq).has_value());

    Reason ipoe{'I', 'P', 'O', 'E'};
    EXPECT_FALSE(decode_halt_reason(ipoe).has_value());
    EXPECT_TRUE(decode_resumption_reason(ipoe).has_value());
}

TEST(TradingReasonTest, CompletelyUnknownCodeReturnsBothNullopt) {
    Reason unknown{'X', 'X', 'X', 'X'};
    EXPECT_FALSE(decode_halt_reason(unknown).has_value());
    EXPECT_FALSE(decode_resumption_reason(unknown).has_value());
}

TEST(TradingReasonTest, SpaceSpaceSpaceSpaceIsNotAvailableInBothTables) {
    // ' ' pads are the REASON_NOT_AVAILABLE sentinel in both tables.
    Reason space{' ', ' ', ' ', ' '};
    auto halt = decode_halt_reason(space);
    auto resume = decode_resumption_reason(space);
    ASSERT_TRUE(halt.has_value());
    ASSERT_TRUE(resume.has_value());
    EXPECT_EQ(*halt, TradingActionReason::REASON_NOT_AVAILABLE);
    EXPECT_EQ(*resume, TradingResumptionReason::REASON_NOT_AVAILABLE);
}

}  // namespace ullfh::core::test
