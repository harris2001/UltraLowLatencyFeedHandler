#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace ullfh::protocols::itch {

/**
 * Common header present in every ITCH 5.0 message (offsets 0–10).
 */
#pragma pack(push, 1)
struct MessageHeader {
    char message_type;         // Offset  0, len 1: message type identifier
    uint16_t stock_locate;     // Offset  1, len 2
    uint16_t tracking_number;  // Offset  3, len 2: internal to NASDAQ
    uint8_t timestamp[6];      // Offset  5, len 6: nanoseconds since midnight
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MessageHeader>);
static_assert(sizeof(MessageHeader) == 11);

enum class EventCode : char {
    START_OF_MESSAGES = 'O',
    START_OF_SYSTEM_HOURS = 'S',
    START_OF_MARKET_HOURS = 'Q',
    END_OF_MARKET_HOURS = 'M',
    END_OF_SYSTEM_HOURS = 'E',
    END_OF_MESSAGES = 'C'
};

/**
 * 1.1 System Event Message – Type 'S'
 */
#pragma pack(push, 1)
struct SystemEventMessage {
    MessageHeader header;  // Offsets 0–10
    EventCode event_code;  // Offset 11, len  1: see EventCode enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<SystemEventMessage>);
static_assert(sizeof(SystemEventMessage) == 12);

// ============================================================================
// 1.2 Stock-Related Messages
// ============================================================================

/*
 * Indicates Listing market or listing market tier for the issue.
 */
enum class MarketCategory : char {
    NASDAQ_GLOBAL_SELECT = 'Q',
    NASDAQ_GLOBAL = 'G',
    NASDAQ_CAPITAL_MARKET = 'S',
    NYSE = 'N',
    NYSE_AMERICAN = 'A',
    NYSE_ARCA = 'P',
    BATS = 'Z',
    INVESTORS = 'V',
    OTHER = ' '
};

/*
 * Shows when the NASDAQ-listed issue is NOT in compliance with Nasdaq continued listing requirements
 */
enum class FSI : char {
    DEFICIENT = 'D',
    DELINQUENT = 'E',
    BANKRUPT = 'Q',
    SUSPENDED = 'S',
    DEFICIENT_AND_BANKRUPT = 'G',
    DEFICIENT_AND_DELINQUENT = 'H',
    DELINQUENT_AND_BANKRUPT = 'J',
    DEFICIENT_DELINQUENT_AND_BANKRUPT = 'K',
    CREATOR_OR_REDEMPTOR_SUSPENDED = 'C',
    NORMAL_DEFAULT = 'N',
    UNKNOWN = ' '
};

/*
 * Yes / No indicator
 */
enum class YesNo : char {
    YES = 'Y',
    NO = 'N',
    NOT_AVAILABLE = ' '
};

/*
 * Appendix D - Issue Classification Values
 */
enum class IssueClassification : char {
    AMERICAN_DEPOSITARY_SHARE = 'A',
    BOND = 'B',
    COMMON_STOCK = 'C',
    DEPOSITORY_RECEIPT = 'F',
    A144 = 'I',
    LIMITED_PARTNERSHIP = 'L',
    NOTES = 'N',
    ORDINARY_SHARE = 'O',
    PREFERRED_STOCK = 'P',
    OTHER_SECURITIES = 'Q',
    RIGHT = 'R',
    SHARES_OF_BENEFICIAL_INTEREST = 'S',
    CONVERTIBLE_DEBENTURE = 'T',
    UNIT = 'U',
    UNITS_BENIF_INT = 'V',
    WARRANT = 'W'
};

/*
 * Appendix E – Issue Sub Type Values
 * Wire format: 2 bytes, right-padded with space for single-character codes.
 */
enum class IssueSubTypeCode : uint8_t {
    PREFERRED_TRUST_SECURITIES,              // A
    ALPHA_INDEX_ETN,                         // AI
    INDEX_BASED_DERIVATIVE,                  // B
    COMMON_SHARES,                           // C
    COMMODITY_BASED_TRUST_SHARES,            // CB
    COMMODITY_FUTURES_TRUST_SHARES,          // CF
    COMMODITY_LINKED_SECURITIES,             // CL
    COMMODITY_INDEX_TRUST_SHARES,            // CM
    COLLATERALIZED_MORTGAGE_OBLIGATION,      // CO
    CURRENCY_TRUST_SHARES,                   // CT
    COMMODITY_CURRENCY_LINKED_SECURITIES,    // CU
    CURRENCY_WARRANTS,                       // CW
    GLOBAL_DEPOSITARY_SHARES,                // D
    ETF_PORTFOLIO_DEPOSITARY_RECEIPT,        // E
    EQUITY_GOLD_SHARES,                      // EG
    ETN_EQUITY_INDEX_LINKED_SECURITIES,      // EI
    NEXTSHARES_EXCHANGE_TRADED_MANAGED_FUND, // EM
    EXCHANGE_TRADED_NOTES,                   // EN
    EQUITY_UNITS,                            // EU
    HOLDRS,                                  // F
    ETN_FIXED_INCOME_LINKED_SECURITIES,      // FI
    ETN_FUTURES_LINKED_SECURITIES,           // FL
    GLOBAL_SHARES,                           // G
    ETF_INDEX_FUND_SHARES,                   // I
    INTEREST_RATE,                           // IR
    INDEX_WARRANT,                           // IW
    INDEX_LINKED_EXCHANGEABLE_NOTES,         // IX
    CORPORATE_BACKED_TRUST_SECURITY,         // J
    CONTINGENT_LITIGATION_RIGHT,             // L
    LIMITED_LIABILITY_COMPANY,               // LL
    EQUITY_BASED_DERIVATIVE,                 // M
    MANAGED_FUND_SHARES,                     // MF
    ETN_MULTI_FACTOR_INDEX_LINKED_SECURITIES,// ML
    MANAGED_TRUST_SECURITIES,                // MT
    NY_REGISTRY_SHARES,                      // N
    OPEN_ENDED_MUTUAL_FUND,                  // O
    PRIVATELY_HELD_SECURITY,                 // P
    POISON_PILL,                             // PP
    PARTNERSHIP_UNITS,                       // PU
    CLOSED_END_FUNDS,                        // Q
    REG_S,                                   // R
    COMMODITY_REDEEMABLE_COMMODITY_LINKED,   // RC
    ETN_REDEEMABLE_FUTURES_LINKED,           // RF
    REIT,                                    // RT
    COMMODITY_REDEEMABLE_CURRENCY_LINKED,    // RU
    SEED,                                    // S
    SPOT_RATE_CLOSING,                       // SC
    SPOT_RATE_INTRADAY,                      // SI
    TRACKING_STOCK,                          // T
    TRUST_CERTIFICATES,                      // TC
    TRUST_UNITS,                             // TU
    PORTAL,                                  // U
    CONTINGENT_VALUE_RIGHT,                  // V
    TRUST_ISSUED_RECEIPTS,                   // W
    WORLD_CURRENCY_OPTION,                   // WC
    TRUST,                                   // X
    OTHER,                                   // Y
    NOT_APPLICABLE,                          // Z
    COUNT
};

/*
 * 2-byte raw wire type for IssueSubType (single-char codes are right-padded with space).
 */
struct IssueSubType {
    char code[2];
    constexpr IssueSubType(char c0, char c1) : code{c0, c1} {}
};
static_assert(std::is_trivially_copyable_v<IssueSubType>);
static_assert(sizeof(IssueSubType) == 2);

constexpr IssueSubType issue_sub_type_table[] = {
    {'A', ' '},  // PREFERRED_TRUST_SECURITIES
    {'A', 'I'},  // ALPHA_INDEX_ETN
    {'B', ' '},  // INDEX_BASED_DERIVATIVE
    {'C', ' '},  // COMMON_SHARES
    {'C', 'B'},  // COMMODITY_BASED_TRUST_SHARES
    {'C', 'F'},  // COMMODITY_FUTURES_TRUST_SHARES
    {'C', 'L'},  // COMMODITY_LINKED_SECURITIES
    {'C', 'M'},  // COMMODITY_INDEX_TRUST_SHARES
    {'C', 'O'},  // COLLATERALIZED_MORTGAGE_OBLIGATION
    {'C', 'T'},  // CURRENCY_TRUST_SHARES
    {'C', 'U'},  // COMMODITY_CURRENCY_LINKED_SECURITIES
    {'C', 'W'},  // CURRENCY_WARRANTS
    {'D', ' '},  // GLOBAL_DEPOSITARY_SHARES
    {'E', ' '},  // ETF_PORTFOLIO_DEPOSITARY_RECEIPT
    {'E', 'G'},  // EQUITY_GOLD_SHARES
    {'E', 'I'},  // ETN_EQUITY_INDEX_LINKED_SECURITIES
    {'E', 'M'},  // NEXTSHARES_EXCHANGE_TRADED_MANAGED_FUND
    {'E', 'N'},  // EXCHANGE_TRADED_NOTES
    {'E', 'U'},  // EQUITY_UNITS
    {'F', ' '},  // HOLDRS
    {'F', 'I'},  // ETN_FIXED_INCOME_LINKED_SECURITIES
    {'F', 'L'},  // ETN_FUTURES_LINKED_SECURITIES
    {'G', ' '},  // GLOBAL_SHARES
    {'I', ' '},  // ETF_INDEX_FUND_SHARES
    {'I', 'R'},  // INTEREST_RATE
    {'I', 'W'},  // INDEX_WARRANT
    {'I', 'X'},  // INDEX_LINKED_EXCHANGEABLE_NOTES
    {'J', ' '},  // CORPORATE_BACKED_TRUST_SECURITY
    {'L', ' '},  // CONTINGENT_LITIGATION_RIGHT
    {'L', 'L'},  // LIMITED_LIABILITY_COMPANY
    {'M', ' '},  // EQUITY_BASED_DERIVATIVE
    {'M', 'F'},  // MANAGED_FUND_SHARES
    {'M', 'L'},  // ETN_MULTI_FACTOR_INDEX_LINKED_SECURITIES
    {'M', 'T'},  // MANAGED_TRUST_SECURITIES
    {'N', ' '},  // NY_REGISTRY_SHARES
    {'O', ' '},  // OPEN_ENDED_MUTUAL_FUND
    {'P', ' '},  // PRIVATELY_HELD_SECURITY
    {'P', 'P'},  // POISON_PILL
    {'P', 'U'},  // PARTNERSHIP_UNITS
    {'Q', ' '},  // CLOSED_END_FUNDS
    {'R', ' '},  // REG_S
    {'R', 'C'},  // COMMODITY_REDEEMABLE_COMMODITY_LINKED
    {'R', 'F'},  // ETN_REDEEMABLE_FUTURES_LINKED
    {'R', 'T'},  // REIT
    {'R', 'U'},  // COMMODITY_REDEEMABLE_CURRENCY_LINKED
    {'S', ' '},  // SEED
    {'S', 'C'},  // SPOT_RATE_CLOSING
    {'S', 'I'},  // SPOT_RATE_INTRADAY
    {'T', ' '},  // TRACKING_STOCK
    {'T', 'C'},  // TRUST_CERTIFICATES
    {'T', 'U'},  // TRUST_UNITS
    {'U', ' '},  // PORTAL
    {'V', ' '},  // CONTINGENT_VALUE_RIGHT
    {'W', ' '},  // TRUST_ISSUED_RECEIPTS
    {'W', 'C'},  // WORLD_CURRENCY_OPTION
    {'X', ' '},  // TRUST
    {'Y', ' '},  // OTHER
    {'Z', ' '},  // NOT_APPLICABLE
};
static_assert(
    static_cast<size_t>(IssueSubTypeCode::COUNT) == std::size(issue_sub_type_table)
);

/*
 * Denotes if an issue or quoting participant  record is set up in a NASDAQ production environment or test environment.
 */
enum class Authenticity : char {
    PRODUCTION = 'P',
    TEST = 'T'
};

/*
 * Indicates the LULD reference price tier for a given issue.
 */
enum class Tier : char {
    TIER_1 = '1',
    TIER_2 = '2',
    NOT_APPLICABLE = ' '
};

/**
 * Stock Directory Message (Section 1.2.1) – Type 'R'
 */
#pragma pack(push, 1)
struct StockDirectoryMessage {
    MessageHeader header;                       // Offsets 0–10
    char stock[8];                              // Offset 11, len  8
    MarketCategory market_category;             // Offset 19, len  1: see MarketCategory enum class
    FSI financial_status_indicator;             // Offset 20, len  1: see FSI enum class
    uint32_t round_lot_size;                    // Offset 21, len  4: number of shares in a round lot
    YesNo round_lots_only;                      // Offset 25, len  1: Yes => Only round lots are allowed
                                                //                    No => Odd and mixed lots are allowed
    IssueClassification issue_classification;   // Offset 26, len  1: see IssueClassification enum class
    IssueSubType issue_sub_type;                // Offset 27, len  2: see IssueSubTypeCode / issue_sub_type_table
    Authenticity authenticity;                  // Offset 29, len  1: see Authenticity enum class
    YesNo short_sale_threshold_indicator;       // Offset 30, len  1: Yes => Restricted under SEC Rule 203(b)(3)
                                                //                    No => Not restricted
    YesNo ipo_flag;                             // Offset 31, len  1: Yes => New IPO, No => Not new
    Tier luld_reference_price_tier;             // Offset 32, len  1: see Tier enum class
    YesNo etp_flag;                             // Offset 33, len  1: Yes => ETP, No => Not ETP
    uint32_t etp_leverage_factor;               // Offset 34, len  4: integral relationship of the ETP to the
                                                //                    underlying index (rounded to the nearest int)
    YesNo inverse_indicator;                    // Offset 38, len  1: Yes => Inverse, No => Not Inverse
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<StockDirectoryMessage>);
static_assert(sizeof(StockDirectoryMessage) == 39);

/*
 * Indicates the current trading state of a stock.
 */
enum class TradingState : char {
    HALTED = 'H',
    PAUSED = 'P',
    QUOTATION_ONLY = 'Q',
    TRADING = 'T'
};

/*
 * Trading Action Reason Codes - Appendix C
 */
enum class TradingActionReason : char {
    HALT_NEWS_PENDING,
    HALT_NEWS_DISSEMINATED,
    SINGLE_SECURITY_TRADING_PAUSE_IN_EFFECT,
    REGULATORY_HALT_EXTRAORDINARY_MARKET_ACTIVITY,
    HALT_ETF,
    TRADING_HALTED_FOR_INFORMATION_REQUESTED_BY_LISTING_MARKET,
    HALT_NON_COMPLIANCE,
    HALT_FILLINGS_NOT_CURRENT,
    HALT_SEC_TRADING_SUSPENSION,
    HALT_REGULATORY_CONCERN,
    OPERATIONS_HALT_CONTACT_MARKET_OPERATIONS,
    VOLATILITY_TRADING_PAUSE,
    VOLATILITY_TRADING_PAUSE_STRADDLE_CONDITION,
    MARKET_WIDE_CIRCUIT_BREAKER_HALT_LEVEL_1,
    MARKET_WIDE_CIRCUIT_BREAKER_HALT_LEVEL_2,
    MARKET_WIDE_CIRCUIT_BREAKER_HALT_LEVEL_3,
    MARKET_WIDE_CIRCUIT_BREAKER_HALT_CARRY_OVER_FROM_PREVIOUS_DAY,
    IPO_ISSUE_NOT_YET_TRADING,
    CORPORATE_ACTION,
    QUOTATION_NOT_AVAILABLE,
    REASON_NOT_AVAILABLE,
    COUNT
};
struct Reason {
    char code[4];  // 4-char alpha code indicating the reason for a trading action

    constexpr Reason(char c0, char c1, char c2, char c3) : code{c0, c1, c2, c3} {}
};
static_assert(std::is_trivially_copyable_v<Reason>);
static_assert(sizeof(Reason) == 4);

constexpr Reason reason_table[] = {
    {'T','1',' ',' '},
    {'T','2',' ',' '},
    {'T','5',' ',' '},
    {'T','6',' ',' '},
    {'T','8',' ',' '},
    {'T','1','2',' '},
    {'H','4',' ',' '},
    {'H','9',' ',' '},
    {'H','1','0',' '},
    {'H','1','1',' '},
    {'O','1',' ',' '},
    {'L','U','D','P'},
    {'L','U','D','S'},
    {'M','W','C','1'},
    {'M','W','C','2'},
    {'M','W','C','3'},
    {'M','W','C','0'},
    {'I','P','O','1'},
    {'M','1',' ',' '},
    {'M','2',' ',' '},
    {' ',' ',' ',' '}
};
static_assert(
    static_cast<size_t>(TradingActionReason::COUNT) == std::size(reason_table)
);

/*
 * Trading Resumption Reason Codes - Appendix C
 * Used in the Reason field of StockTradingActionMessage when TradingState is TRADING or QUOTATION_ONLY.
 */
enum class TradingResumptionReason : uint8_t {
    NEWS_AND_RESUMPTION_TIMES,                          // T3
    SINGLE_SECURITY_TRADING_PAUSE_QUOTATION_ONLY,       // T7
    QUALIFICATIONS_ISSUES_REVIEWED_RESOLVED,            // R4
    FILING_REQUIREMENTS_SATISFIED_RESOLVED,             // R9
    ISSUER_NEWS_NOT_FORTHCOMING,                        // C3
    QUALIFICATIONS_HALT_ENDED_MAINTENANCE_MET,          // C4
    QUALIFICATIONS_HALT_CONCLUDED_FILINGS_MET,          // C9
    TRADE_HALT_CONCLUDED_BY_OTHER_REGULATORY_AUTHORITY, // C11
    MARKET_WIDE_CIRCUIT_BREAKER_RESUMPTION,             // MWCQ
    NEW_ISSUE_AVAILABLE,                                // R1
    ISSUE_AVAILABLE,                                    // R2
    IPO_SECURITY_RELEASED_FOR_QUOTATION,                // IPOQ
    IPO_SECURITY_POSITIONING_WINDOW_EXTENSION,          // IPOE
    REASON_NOT_AVAILABLE,                               // (space)
    COUNT
};

constexpr Reason resumption_reason_table[] = {
    {'T','3',' ',' '},  // NEWS_AND_RESUMPTION_TIMES
    {'T','7',' ',' '},  // SINGLE_SECURITY_TRADING_PAUSE_QUOTATION_ONLY
    {'R','4',' ',' '},  // QUALIFICATIONS_ISSUES_REVIEWED_RESOLVED
    {'R','9',' ',' '},  // FILING_REQUIREMENTS_SATISFIED_RESOLVED
    {'C','3',' ',' '},  // ISSUER_NEWS_NOT_FORTHCOMING
    {'C','4',' ',' '},  // QUALIFICATIONS_HALT_ENDED_MAINTENANCE_MET
    {'C','9',' ',' '},  // QUALIFICATIONS_HALT_CONCLUDED_FILINGS_MET
    {'C','1','1',' '},  // TRADE_HALT_CONCLUDED_BY_OTHER_REGULATORY_AUTHORITY
    {'M','W','C','Q'},  // MARKET_WIDE_CIRCUIT_BREAKER_RESUMPTION
    {'R','1',' ',' '},  // NEW_ISSUE_AVAILABLE
    {'R','2',' ',' '},  // ISSUE_AVAILABLE
    {'I','P','O','Q'},  // IPO_SECURITY_RELEASED_FOR_QUOTATION
    {'I','P','O','E'},  // IPO_SECURITY_POSITIONING_WINDOW_EXTENSION
    {' ',' ',' ',' '},  // REASON_NOT_AVAILABLE
};
static_assert(
    static_cast<size_t>(TradingResumptionReason::COUNT) == std::size(resumption_reason_table)
);

// ============================================================================
// Reason code decode functions → see include/protocols/itch/ITCHDecoder.hpp
// (constexpr Fibonacci hash map + decode_halt_reason / decode_resumption_reason)
// ============================================================================

/**
 * Stock Trading Action Message (Section 1.2.2) – Type 'H'
 */
#pragma pack(push, 1)
struct StockTradingActionMessage {
    MessageHeader header;        // Offsets 0–10
    char stock[8];               // Offset 11, len 8
    TradingState trading_state;  // Offset 19, len 1: see TradingState enum class
    char reserved;               // Offset 20, len 1
    Reason reason;               // Offset 21, len 4: see TradingActionReason (halt) / TradingResumptionReason (resume)
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<StockTradingActionMessage>);
static_assert(sizeof(StockTradingActionMessage) == 25);

enum class PriceTest : char {
    NO_PRICE_TEST = '0',
    PRICE_TEST_RESTRICTION_IN_EFFECT_INTRA_DAY_PRICE_DROP = '1',
    PRICE_TEST_RESTRICTION_REMAINS_IN_EFFECT = '2'
};

/**
 * Reg SHO Short Sale Price Test Restricted Indicator (Section 1.2.3) – Type 'Y'
 */
#pragma pack(push, 1)
struct RegSHOShortSalePriceTestRestrictedMessage {
    MessageHeader header;     // Offsets 0–10
    char stock[8];            // Offset 11, len 8
    PriceTest regsho_action;  // Offset 19, len 1
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<RegSHOShortSalePriceTestRestrictedMessage>);
static_assert(sizeof(RegSHOShortSalePriceTestRestrictedMessage) == 20);

enum class IsPrimary : char {
    PRIMARY = 'Y',
    NON_PRIMARY = 'N'
};

enum class MarketMakerMode : char {
    NORMAL = 'N',
    PASSIVE = 'P',
    SYNDICATE = 'S',
    PRE_SYNDICATE = 'R',
    PENALTY = 'L'
};

enum class MarketParticipantState : char {
    ACTIVE = 'A',
    EXCUSED_OR_WITHDRAWN = 'E',
    WITHDRAWN = 'W',
    SUSPENDED = 'S',
    DELETED = 'D'
};

/**
 * Market Participant Position Message (Section 1.2.4) – Type 'L'
 * Indicates the current quoting status of a market participant at Nasdaq.
 */
#pragma pack(push, 1)
struct MarketParticipantPositionMessage {
    MessageHeader header;                             // Offsets 0–10
    char mpid[4];                                     // Offset 11, len 4: market participant identifier
    char stock[8];                                    // Offset 15, len 8
    IsPrimary primary_market_maker;                   // Offset 23, len 1: see IsPrimary enum class
    MarketMakerMode market_maker_mode;                // Offset 24, len 1: see MarketMakerMode enum class
    MarketParticipantState market_participant_state;  // Offset 25, len 1: see MarketParticipantState enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MarketParticipantPositionMessage>);
static_assert(sizeof(MarketParticipantPositionMessage) == 26);

/**
 * A strongly-typed wrapper around a raw integer price field.
 *
 * @tparam T : Underlying integer type (e.g. uint32_t, uint64_t)
 * @tparam Precision : Number of decimal places (4 or 8 per ITCH spec)
 *
 * Wire layout is identical to T — no extra bytes.
 * Precision is a compile-time constant; no runtime storage needed.
 */
template <typename T, int Precision>
struct Price {
    T raw;  // Raw integer value as received on the wire

    [[nodiscard]] double get() const noexcept {
        constexpr double scale = [] {
            double s = 1.0;
            for (int i = 0; i < Precision; ++i) s *= 10.0;
            return s;
        }();
        return static_cast<double>(raw) / scale;
    }
};
static_assert(std::is_trivially_copyable_v<Price<uint32_t, 4>>);
static_assert(std::is_trivially_copyable_v<Price<uint64_t, 8>>);
static_assert(sizeof(Price<uint32_t, 4>) == sizeof(uint32_t));
static_assert(sizeof(Price<uint64_t, 8>) == sizeof(uint64_t));

using Price4 = Price<uint32_t, 4>;  // price × 10'000
using Price8 = Price<uint64_t, 8>;  // price × 100'000'000

/**
 * MWCB Decline Level Message (Section 1.2.5.1) – Type 'V'
 * Disseminates the daily MWCB halt threshold price levels.
 */
#pragma pack(push, 1)
struct MWCBDeclineLevelMessage {
    MessageHeader header;  // Offsets 0–10
    Price8 level_1;        // Offset 11, len 8: Price 8
    Price8 level_2;        // Offset 19, len 8: Price 8
    Price8 level_3;        // Offset 27, len 8: Price 8
};
#pragma pack(pop)
static_assert(std::is_trivially_copyable_v<MWCBDeclineLevelMessage>);
static_assert(sizeof(MWCBDeclineLevelMessage) == 35);

enum class Levels : char { LEVEL_1 = '1', LEVEL_2 = '2', LEVEL_3 = '3' };

/**
 * MWCB Status Message (Section 1.2.5.2) – Type 'W'
 * Indicates that a Market-Wide Circuit Breaker decline level has been breached.
 */
#pragma pack(push, 1)
struct MWCBStatusMessage {
    MessageHeader header;   // Offsets 0–10
    Levels breached_level;  // Offset 11, len 1: see Levels enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MWCBStatusMessage>);
static_assert(sizeof(MWCBStatusMessage) == 12);

enum class IPOQuotationReleaseQualifier : char { ANTICIPATED = 'A', IPO_CANCELLED = 'C' };
/**
 * IPO Quoting Period Update (Section 1.2.6) – Type 'K'
 * Indicates anticipated IPO quotation release time and price.
 */
#pragma pack(push, 1)
struct IPOQuotingPeriodUpdateMessage {
    MessageHeader header;                 // Offsets 0–10
    char stock[8];                        // Offset 11, len 8
    uint32_t ipo_quotation_release_time;  // Offset 19, len 4: seconds since midnight
    IPOQuotationReleaseQualifier
        ipo_quotation_release_qualifier;  // Offset 23, len 1: see IPOQuotationReleaseQualifier enum class
    Price4 ipo_price;                     // Offset 24, len 4: Price 4
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<IPOQuotingPeriodUpdateMessage>);
static_assert(sizeof(IPOQuotingPeriodUpdateMessage) == 28);

/**
 * LULD Auction Collar Message (Section 1.2.7) – Type 'J'
 * Indicates the auction collar thresholds within which a stock may trade
 * during a LULD trading pause.
 */
#pragma pack(push, 1)
struct LULDAuctionCollarMessage {
    MessageHeader header;                   // Offsets 0–10
    char stock[8];                          // Offset 11, len 8
    Price4 auction_collar_reference_price;  // Offset 19, len 4: Price 4
    Price4 upper_auction_collar_price;      // Offset 23, len 4: Price 4
    Price4 lower_auction_collar_price;      // Offset 27, len 4: Price 4
    uint32_t auction_collar_extension;      // Offset 31, len 4: number of Auction Collar Extensions
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<LULDAuctionCollarMessage>);
static_assert(sizeof(LULDAuctionCollarMessage) == 35);

enum class MarketCode : char { NASDAQ = 'Q', BX = 'B', PSX = 'X' };

enum class OperationalHaltAction : char { OPERATIONALLY_HALTED = 'H', TRADING_RESUMED = 'T' };
/**
 * Operational Halt Message (Section 1.2.8) – Type 'h'
 * Indicates an operational halt or trading resumption for a Nasdaq-listed security.
 */
#pragma pack(push, 1)
struct OperationalHaltMessage {
    MessageHeader header;                           // Offsets 0–10
    char stock[8];                                  // Offset 11, len 8
    MarketCode market_code;                         // Offset 19, len 1: see MarketCode enum class
    OperationalHaltAction operational_halt_action;  // Offset 20, len 1: see OperationalHaltAction enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OperationalHaltMessage>);
static_assert(sizeof(OperationalHaltMessage) == 21);

// ============================================================================
// Add Order Messages (Section 1.3)
// ============================================================================

enum class BuySellIndicator : char { BUY = 'B', SELL = 'S' };
/**
 * Add Order – No MPID Attribution (Section 1.3.1) – Type 'A'
 * Adds a new order to the limit order book.
 */
#pragma pack(push, 1)
struct AddOrderMessage {
    MessageHeader header;                 // Offsets 0–10
    uint64_t order_reference;             // Offset 11, len 8: unique order reference number
    BuySellIndicator buy_sell_indicator;  // Offset 19, len 1: see BuySellIndicator enum class
    uint32_t shares;                      // Offset 20, len 4
    char stock[8];                        // Offset 24, len 8
    Price4 price;                         // Offset 32, len 4: Price4
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<AddOrderMessage>);
static_assert(sizeof(AddOrderMessage) == 36);

/**
 * Add Order – MPID Attribution (Section 1.3.2) – Type 'F'
 */
#pragma pack(push, 1)
struct AddOrderWithMPIDMessage {
    MessageHeader header;                 // Offsets 0–10
    uint64_t order_reference;             // Offset 11, len 8
    BuySellIndicator buy_sell_indicator;  // Offset 19, len 1: see BuySellIndicator enum class
    uint32_t shares;                      // Offset 20, len 4
    char stock[8];                        // Offset 24, len 8
    Price4 price;                         // Offset 32, len 4: Price4
    char attribution[4];                  // Offset 36, len 4: MPID of the entered order
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<AddOrderWithMPIDMessage>);
static_assert(sizeof(AddOrderWithMPIDMessage) == 40);

// ============================================================================
// Modify Order Messages (Section 1.4)
// ============================================================================

/**
 * Order Executed Message (Section 1.4.1) – Type 'E'
 * Indicates a visible order on the book was executed in part or in full.
 */
#pragma pack(push, 1)
struct OrderExecutedMessage {
    MessageHeader header;      // Offsets 0–10
    uint64_t order_reference;  // Offset 11, len 8
    uint32_t executed_shares;  // Offset 19, len 4
    uint64_t match_number;     // Offset 23, len 8: unique match identifier
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OrderExecutedMessage>);
static_assert(sizeof(OrderExecutedMessage) == 31);

enum class Printable : char { PRINTABLE = 'Y', NON_PRINTABLE = 'N' };
/**
 * Order Executed With Price Message (Section 1.4.2) – Type 'C'
 * Like 'E' but used when the execution price differs from the order price.
 */
#pragma pack(push, 1)
struct OrderExecutedWithPriceMessage {
    MessageHeader header;      // Offsets 0–10
    uint64_t order_reference;  // Offset 11, len 8
    uint32_t executed_shares;  // Offset 19, len 4
    uint64_t match_number;     // Offset 23, len 8
    Printable printable;       // Offset 31, len 1: see Printable enum class
    Price4 execution_price;    // Offset 32, len 4: Price4
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OrderExecutedWithPriceMessage>);
static_assert(sizeof(OrderExecutedWithPriceMessage) == 36);

/**
 * Order Cancel Message (Section 1.4.3) – Type 'X'
 * Partial cancellation of a visible order.
 */
#pragma pack(push, 1)
struct OrderCancelMessage {
    MessageHeader header;       // Offsets 0–10
    uint64_t order_reference;   // Offset 11, len 8
    uint32_t cancelled_shares;  // Offset 19, len 4: number of shares removed
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OrderCancelMessage>);
static_assert(sizeof(OrderCancelMessage) == 23);

/**
 * Order Delete Message (Section 1.4.4) – Type 'D'
 * Full removal of a visible order.
 */
#pragma pack(push, 1)
struct OrderDeleteMessage {
    MessageHeader header;      // Offsets 0–10
    uint64_t order_reference;  // Offset 11, len 8
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OrderDeleteMessage>);
static_assert(sizeof(OrderDeleteMessage) == 19);

/**
 * Order Replace Message (Section 1.4.5) – Type 'U'
 * Replaces an existing visible order with a new one at a new price/quantity.
 * The original order is implicitly cancelled.
 */
#pragma pack(push, 1)
struct OrderReplaceMessage {
    MessageHeader header;               // Offsets 0–10
    uint64_t original_order_reference;  // Offset 11, len 8: reference of the order being replaced
    uint64_t new_order_reference;       // Offset 19, len 8: reference of the replacement order
    uint32_t shares;                    // Offset 27, len 4: new total display quantity
    Price4 price;                       // Offset 31, len 4: Price4
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<OrderReplaceMessage>);
static_assert(sizeof(OrderReplaceMessage) == 35);

// ============================================================================
// Trade Messages (Section 1.5)
// ============================================================================

/**
 * Trade Message – Non-Cross (Section 1.5.1) – Type 'P'
 * Provides execution details for a trade that occurred on the Nasdaq Market Center that is not part of a cross.
 */
#pragma pack(push, 1)
struct TradeMessage {
    MessageHeader header;                 // Offsets 0–10
    uint64_t order_reference;             // Offset 11, len 8: reference of the order that was matched
    BuySellIndicator buy_sell_indicator;  // Offset 19, len 1: see BuySellIndicator enum class
    uint32_t shares;                      // Offset 20, len 4
    char stock[8];                        // Offset 24, len 8
    Price4 price;                         // Offset 32, len 4: Price4
    uint64_t match_number;                // Offset 36, len 8
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<TradeMessage>);
static_assert(sizeof(TradeMessage) == 44);

enum class CrossType : char { OPENING = 'O', CLOSING = 'C', IPO_HALTED = 'H', EXTENDED_TRADING_CLOSE = 'E' };

/**
 * Cross Trade Message (Section 1.5.2) – Type 'Q'
 * Published for NASDAQ opening, closing, and IPO cross executions.
 */
#pragma pack(push, 1)
struct CrossTradeMessage {
    MessageHeader header;   // Offsets 0–10
    uint64_t shares;        // Offset 11, len 8: number of shares matched in the cross
    char stock[8];          // Offset 19, len 8
    Price4 cross_price;     // Offset 27, len 4: Price4
    uint64_t match_number;  // Offset 31, len 8
    CrossType cross_type;   // Offset 39, len 1: see CrossType enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<CrossTradeMessage>);
static_assert(sizeof(CrossTradeMessage) == 40);

/**
 * Broken Trade / Order Execution Message (Section 1.5.3) – Type 'B'
 * Signals that a previously published trade should be treated as cancelled.
 */
#pragma pack(push, 1)
struct BrokenTradeMessage {
    MessageHeader header;   // Offsets 0–10
    uint64_t match_number;  // Offset 11, len 8: match number of the trade being broken
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<BrokenTradeMessage>);
static_assert(sizeof(BrokenTradeMessage) == 19);

// ============================================================================
// Net Order Imbalance Indicator (Section 1.6)
// ============================================================================

enum class PriceVariationIndicator : char {
    LESS_THAN_1_PERCENT = 'L',
    BETWEEN_1_AND_1_99_PERCENT = '1',
    BETWEEN_2_AND_2_99_PERCENT = '2',
    BETWEEN_3_AND_3_99_PERCENT = '3',
    BETWEEN_4_AND_4_99_PERCENT = '4',
    BETWEEN_5_AND_5_99_PERCENT = '5',
    BETWEEN_6_AND_6_99_PERCENT = '6',
    BETWEEN_7_AND_7_99_PERCENT = '7',
    BETWEEN_8_AND_8_99_PERCENT = '8',
    BETWEEN_9_AND_9_99_PERCENT = '9',
    BETWEEN_10_AND_10_99_PERCENT = 'A',
    BETWEEN_20_AND_29_99_PERCENT = 'B',
    GREATER_THAN_OR_EQUAL_TO_30_PERCENT = 'C',
    CANNOT_BE_CALCULATED = ' '
};

enum class ImbalanceDirection : char { BUY = 'B', SELL = 'S', NO_IMBALANCE = 'N', INSUFFICIENT_ORDERS = 'O', PAUSED = 'P' };

/**
 * NOII Message (Section 1.6) – Type 'I'
 * Disseminates order imbalance information in the pre-market and post-market
 * periods and during the Nasdaq Opening Cross.
 */
#pragma pack(push, 1)
struct NOIIMessage {
    MessageHeader header;       // Offsets 0–10
    uint64_t paired_shares;     // Offset 11, len  8: shares eligible to be matched at Current Reference Price
    uint64_t imbalance_shares;  // Offset 19, len  8: shares not paired at Current Reference Price
    ImbalanceDirection imbalance_direction;             // Offset 27, len  1: see ImbalanceDirection enum class
    char stock[8];                                      // Offset 28, len  8
    Price4 far_price;                                   // Offset 36, len  4: Price4
    Price4 near_price;                                  // Offset 40, len  4: Price4
    Price4 current_reference_price;                     // Offset 44, len  4: Price4
    CrossType cross_type;                               // Offset 48, len  1: see CrossType enum class
    PriceVariationIndicator price_variation_indicator;  // Offset 49, len  1: see PriceVariationIndicator enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<NOIIMessage>);
static_assert(sizeof(NOIIMessage) == 50);

// ============================================================================
// Retail Price Improvement Indicator (Section 1.7)
// ============================================================================

enum class Interest : char { RPI_ON_BUY = 'B', RPI_ON_SELL = 'S', RPI_ON_BOTH_SIDES = 'A', NO_RPI = 'N' };
/**
 * RPII Message (Section 1.7) – Type 'N'
 * Identifies retail interest in a Nasdaq-listed security and the side(s) of the market.
 */
#pragma pack(push, 1)
struct RPIIMessage {
    MessageHeader header;    // Offsets 0–10
    char stock[8];           // Offset 11, len 8
    Interest interest_flag;  // Offset 19, len 1: see Interest enum class
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<RPIIMessage>);
static_assert(sizeof(RPIIMessage) == 20);

// ============================================================================
// Direct Listing with Capital Raise Price Discovery Message (Section 1.8)
// ============================================================================

enum class OpenEligibilityStatus : char { NOT_ELIGIBLE = 'N', ELIGIBLE = 'Y' };

/**
 * Direct Listing with Capital Raise Price Discovery Message (Section 1.8) – Type 'O'
 * Disseminated once per second after the DLCR volatility test has successfully passed.
 */
#pragma pack(push, 1)
struct DLCRPriceDiscoveryMessage {
    MessageHeader header;                           // Offsets 0–10
    char stock[8];                                  // Offset 11, len 8: stock symbol, right padded with spaces
    OpenEligibilityStatus open_eligibility_status;  // Offset 19, len 1: see OpenEligibilityStatus enum class
    Price4 minimum_allowable_price;                 // Offset 20, len 4: 20% below Registration Statement Lower Price
    Price4 maximum_allowable_price;                 // Offset 24, len 4: 80% above Registration Statement Highest Price
    Price4 near_execution_price;                    // Offset 28, len 4: current reference price when DLCR volatility test passed
    uint64_t near_execution_time;                   // Offset 32, len 8: time at which near execution price was set
    Price4 lower_price_range_collar;                // Offset 40, len 4: Lower Auction Collar Threshold (10% below near execution price)
    Price4 upper_price_range_collar;                // Offset 44, len 4: Upper Auction Collar Threshold (10% above near execution price)
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<DLCRPriceDiscoveryMessage>);
static_assert(sizeof(DLCRPriceDiscoveryMessage) == 48);

// ============================================================================
// All messages must be trivially copyable for zero-copy parsing
// ============================================================================

}  // namespace ullfh::protocols::itch
