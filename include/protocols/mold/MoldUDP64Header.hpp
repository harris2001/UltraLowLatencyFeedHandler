#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/itch/ITCHMessages.hpp"

namespace ullfh::protocols::mold {

using itch::BeU16;
using itch::BeU64;

/**
 * MoldUDP64 Downstream Packet Header (spec 3.1)
 *
 * This is present exactly before zero or more Message Blocks in every downstream packet received
 */
#pragma pack(push, 1)
struct MoldUDP64Header {
    char session[10];       // Offset 0, len 10 - ANUM session identifier (space-padded)
    BeU64 sequence_number;  // Offset 10, len 8 - sequence number of the first message
    BeU16 message_count;    // Offset 18, len 2 - number of Message Blocks in this packet
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MoldUDP64Header>);
static_assert(sizeof(MoldUDP64Header) == 20, "MoldUDP64 downstream header must be 20 bytes per spec 3.1");

/**
 * MoldUDP64 Message Block header (spec 3.2)
 *
 * Immediately follows the MoldUDP64Header and precedes the ITCH payload. 
 * Each block starts with a 2-byte big-endian message length and then `message_length` bytes of ITCH data. 
 */
#pragma pack(push, 1)
struct MoldMessageBlock {
    BeU16 message_length; // Specifies how many bytes follow
    // Message Data follows immediately: const uint8_t data[message_length];
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MoldMessageBlock>);
static_assert(sizeof(MoldMessageBlock) == 2, "MoldUDP64 message block header must be 2 bytes per spec 3.2");

/**
 * Special values for MoldUDP64Header::message_count (spec 3.1.2)
 */
inline constexpr uint16_t k_heartbeat = 0x00'00u;       // Packet is a heartbeat (no messages)
inline constexpr uint16_t k_end_of_session = 0xFF'FFu;  // Session has ended

/**
 * MoldUDP64 Request Packet (spec 4)
 *
 * In case we missed messages and we want to request retransmission we need to send the following request packet
 * Note that we are currently not using this since we only implement gap detection but we <define it here 
 * for completeness
 */
#pragma pack(push, 1)
struct MoldUDP64RequestPacket {
    char session[10];               // Offset 0, len 10
    BeU64 sequence_number;          // Offset 10, len 8 - first requested sequence number
    BeU16 requested_message_count;  // Offset 18, len 2 - number of messages requested
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable_v<MoldUDP64RequestPacket>);
static_assert(sizeof(MoldUDP64RequestPacket) == 20);

}  // namespace ullfh::protocols::mold
