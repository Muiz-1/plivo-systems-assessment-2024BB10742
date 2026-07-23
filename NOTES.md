# Systems Architecture & Design Notes

## Problem Overview
The objective is to optimize a UDP streaming protocol to achieve minimal playout delay and deadline misses (<1.00%) over a lossy, high-jitter network while adhering strictly to a **2.00x bandwidth overhead cap**.

---

## Key Design & Reasoning Decisions

### 1. Mathematical Overhead Limit & Packet Budgeting
- **Frame Size**: Each audio frame is 160 bytes of payload + 4 bytes sequence header = **164 bytes per packet**.
- **Bandwidth Cap**: For 1,500 frames (240,000 bytes base payload), a 2.00x cap allows a maximum of **480,000 bytes**.
- **Max Packet Budget**: 
  $$\lfloor 480,000 \text{ bytes} / 164 \text{ bytes} \rfloor = 2,926 \text{ packets}$$
- **Duplication Strategy ($19/20$ Ratio)**:
  - We transmit a duplicate packet for 19 out of every 20 frames.
  - Total packets sent: $1,500 + \lfloor 1,500 \times \frac{19}{20} \rfloor = 2,925 \text{ packets}$.
  - Total byte overhead: $2,925 \times 164 = 479,700 \text{ bytes}$ (**1.99875x overhead**).
  - This utilizes **99.9%** of our allowed budget without crossing the $2.00\text{x}$ cap.

### 2. Immediate Duplication vs. Staggered Redundancy
- **Experimented with Staggered Redundancy**: Piggybacking frame $N-1$ onto frame $N$ to defeat micro-burst drops.
- **Result**: Miss rate increased from 1.00% to 2.87% on Profile B at 80 ms.
- **Takeaway**: Holding duplicates introduces a 20 ms delay penalty before transmission. Under tight playout deadlines (e.g., 80 ms), immediate duplication gives packets the maximum physical window to arrive safely.

### 3. Receiver Deduplication Strategy
- Implemented an $O(1)$ bitset table (`seen_bitset`) in `receiver.c` tracking sequence numbers up to $1,048,576$.
- Eliminates duplicate packet forwarding to the audio player without memory overhead or dynamic allocation delays.

### 4. Kernel Socket Tuning
- Expanded `SO_RCVBUF` and `SO_SNDBUF` to 1 MB to absorb burst transmissions on `localhost`.
- Configured `IPTOS_LOWDELAY` on send sockets to prioritize UDP frame processing.

---

## Final Performance Summary
- **Profile A (`profiles/A.json`)**: Playout Delay **40 ms** | Miss Rate **0.87%** (13 misses) | Overhead **2.00x** (**VALID**)
- **Profile B (`profiles/B.json`)**: Playout Delay **85 ms** | Miss Rate **0.80%** (12 misses) | Overhead **2.00x** (**VALID**)
