#pragma once

#include <cstdint>
#include <type_traits>

namespace ullfh::protocols::itch {


/**
 * Common header present in every ITCH 5.0 message (offsets 0–10).
 */
#pragma pack(push, 1)
struct MessageHeader {
    char     message_type;     // Offset  0, len 1: message type identifier
    uint16_t stock_locate;     // Offset  1, len 2
    uint16_t tracking_number;  // Offset  3, len 2: internal to NASDAQ
    uint8_t  timestamp[6];     // Offset  5, len 6: nanoseconds since midnight
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MessageHeader>);
static_assert(sizeof(MessageHeader) == 11);

enum EventCode : char {
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
    MessageHeader header;      // Offsets 0–10
    EventCode event_code;      // Offset 11, len  1: see EventCode enum
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
enum MarketCategory : char {
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
enum FSI : char {
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
enum Yes_No : char {
    YES = 'Y',
    NO = 'N',
    NOT_AVAILABLE = ' '
};

/*
 * Appendix D - Issue Classification Values
 */
enum IssueClassification : char {
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
 * Denotes if an issue or quoting participant  record is set up in a NASDAQ production environment or test environment.
 */
enum Authenticity : char {
    PRODUCTION = 'P',
    TEST = 'T'
};

/*
 * Indicates the LULD reference price tier for a given issue.
 */
enum Tier : char {
    TIER_1 = '1',
    TIER_2 = '2',
    NOT_APPLICABLE = ' '
};

/**
 * Stock Directory Message – Type 'R'
 */
#pragma pack(push, 1)
struct StockDirectoryMessage {
    MessageHeader header;                           // Offsets 0–10
    char stock[8];                                  // Offset 11, len  8
    MarketCategory market_category;                 // Offset 19, len  1: see MarketCategory enum 
    FSI financial_status_indicator;                 // Offset 20, len  1: see FSI enum
    uint32_t round_lot_size;                        // Offset 21, len  4: number of shares in a round lot
    Yes_No round_lots_only;                         // Offset 25, len  1: Yes => Only round lots are allowed
                                                    //                    No => Odd and mixed lots are allowed 
    IssueClassification issue_classification;       // Offset 26, len  1: see IssueClassification enum
    char issue_sub_type[2];                         // Offset 27, len  2: 2-char alpha
    Authenticity authenticity;                      // Offset 29, len  1: see Authenticity enum
    Yes_No short_sale_threshold_indicator;          // Offset 30, len  1: Yes => Restricted under SEC Rule 203(b)(3)
                                                    //                    No => Not restricted 
    Yes_No ipo_flag;                                // Offset 31, len  1: Yes => New IPO, No => Not new 
    Tier luld_reference_price_tier;                 // Offset 32, len  1: see Tier enum 
    Yes_No etp_flag;                                // Offset 33, len  1: Yes => ETP, No => Not ETP
    uint32_t etp_leverage_factor;                   // Offset 34, len  4: integral relationship of the ETP to the 
                                                    //                    underlying index (rounded to the nearest int) 
    Yes_No inverse_indicator;                       // Offset 38, len  1: Yes => Inverse, No => Not Inverse
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<StockDirectoryMessage>);
static_assert(sizeof(StockDirectoryMessage) == 39);

/*
 * Indicates the current trading state of a stock.
 */
enum TradingState : char {
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


/**
 * Stock Trading Action Message (Section 1.2.2) – Type 'H'
 */
#pragma pack(push, 1)
struct StockTradingActionMessage {
    MessageHeader header;       // Offsets 0–10
    char stock[8];              // Offset 11, len 8
    TradingState trading_state; // Offset 19, len 1: see TradingState enum
    char reserved;              // Offset 20, len 1
    Reason reason;              // Offset 21, len 4: see TradingActionReason namespace
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<StockTradingActionMessage>);
static_assert(sizeof(StockTradingActionMessage) == 25);


enum PriceTest : char {
    NO_PRICE_TEST = '0',
    PRICE_TEST_RESTRICTION_IN_EFFECT_INTRA_DAY_PRICE_DROP = '1',
    PRICE_TEST_RESTRICTION_REMAINS_IN_EFFECT = '2'
};

/**
 * Reg SHO Short Sale Price Test Restricted Indicator (Section 1.2.3) – Type 'Y'
 */
#pragma pack(push, 1)
struct RegSHOShortSalePriceTestRestrictedMessage {
    MessageHeader header;           // Offsets 0–10
    char stock[8];                  // Offset 11, len 8
    PriceTest regsho_action;        // Offset 19, len 1
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<RegSHOShortSalePriceTestRestrictedMessage>);
static_assert(sizeof(RegSHOShortSalePriceTestRestrictedMessage) == 20);

enum IsPrimary : char {
    PRIMARY = 'Y',
    NON_PRIMARY = 'N'
};

enum MarketMakerMode : char {
    NORMAL = 'N',
    PASSIVE = 'P',
    SYNDICATE = 'S',
    PRE_SYNDICATE = 'R',
    PENALTY = 'L'
};

enum MarketParticipantState : char {
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
    MessageHeader header;                               // Offsets 0–10
    char mpid[4];                                       // Offset 11, len 4: market participant identifier
    char stock[8];                                      // Offset 15, len 8
    IsPrimary primary_market_maker;                     // Offset 23, len 1: see IsPrimary enum 
    MarketMakerMode market_maker_mode;                  // Offset 24, len 1: see MarketMakerMode enum
    MarketParticipantState market_participant_state;    // Offset 25, len 1: see MarketParticipantState enum
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MarketParticipantPositionMessage>);
static_assert(sizeof(MarketParticipantPositionMessage) == 26);

// ============================================================================
// All messages must be trivially copyable for zero-copy parsing
// ============================================================================

}  // namespace ullfh::protocols::itch
