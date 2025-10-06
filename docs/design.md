# Ultra Low Latency Feed Handler - Design Document
## Overview
The objective of this project is to develop a high-performance feed handler capable of parsing and processing NASDAQ ToatalView-ITCH 5.0 market data feeds in real time.

## System Architecture

1. UDP Multicast:
   - The exchange feed will produce a binary ITCH 5.0 message stream over UDP multicast.
2. Feed Handler:
   - Our feed handler will be listening to a specified multicast group and port.
3. ITCH Parser:
   - Our parser will parse incoming ITCH 5.0 messages.
4. Ring Buffer:
   - Parsed messages will be stored in a ring buffer so that we can efficiently access them.
   - This will be implemented using a lock-free data structure to minimize latency.
5. Event Consumer:
   - Our event consumer will read from the ring buffer and process the messages.
   - We can then use these data for logging, analytics, or we can further forward them to another system for further processing.

### Example ITCH 5.0 Message
Here is an example of an Add Order message in ITCH format:
```
0x410000000000000000000003ea00000000000500cd4200000001000000000000000100000000
```
This message can be broken down as follows:
```
|==================|========|========|==============|========================|=========|============================|
| Field Name       | Offset | Length | Format       | Binary Representation  | Decoded | Notes                      |
|==================|========|========|==============|========================|=========|============================|
| Message Type     | 0      | 1      | Alphanumeric | 0x41                   | 'A'     | Add Order                  |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Timestamp        | 1      | 8      | Numeric      | 0x0000000000000000     | 0       | Time in nanoseconds        |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Order Reference  | 9      | 4      | Numeric      | 0x000003ea             | 1002    |                            |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Transaction ID   | 13     | 4      | Numeric      | 0x00000000             | 0       |                            |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Order Book ID    | 17     | 4      | Numeric      | 0x000500cd             | 327885  |                            |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Side             | 21     | 1      | Alphanumeric | 0x42                   | 'B'     | 'B' for Buy, 'S' for Sell  |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Quantity         | 22     | 4      | Numeric      | 0x00000001             | 1       |                            |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Price            | 26     | 8      | Price        | 0x0000000000000001     | 1       |                            |
|------------------|--------|--------|--------------|------------------------|---------|----------------------------|
| Yield            | 34     | 4      | Price        | 0x00000000             | 0       |                            |
|==================|========|========|==============|========================|=========|============================|
```
