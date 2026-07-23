# Assessment Benchmark Run Log

## Baseline (Unmodified Code)
- **Profile A (`profiles/A.json`, Delay: 40ms)**:
  - Miss Rate: 5.13% (77 misses)
  - Overhead: 1.02x
  - Result: **INVALID**

---

## Iteration 1: Selective Immediate Duplication (15/16 Packets)
- **Profile A (`profiles/A.json`, Delay: 40ms)**:
  - Miss Rate: 0.87% (13 misses)
  - Overhead: 1.99x
  - Result: **VALID**

- **Profile B (`profiles/B.json`, Delay: 85ms)**:
  - Miss Rate: 0.73% (11 misses)
  - Overhead: 1.99x
  - Result: **VALID**

- **Profile B (`profiles/B.json`, Delay: 80ms)**:
  - Miss Rate: 1.00% (15 misses)
  - Overhead: 1.99x
  - Result: **VALID**

---

## Iteration 2: Max-Density Duplication (19/20 Packets)
- **Profile B (`profiles/B.json`, Delay: 85ms)**:
  - Miss Rate: 0.80% (12 misses)
  - Overhead: 2.00x
  - Result: **VALID**

- **Profile A (`profiles/A.json`, Delay: 40ms)**:
  - Miss Rate: 0.87% (13 misses)
  - Overhead: 2.00x
  - Result: **VALID**

---

## Iteration 3: Socket Buffer & Low-Latency Tuning
- **Profile B (`profiles/B.json`, Delay: 80ms)**:
  - Miss Rate: 1.00% (15 misses)
  - Overhead: 2.00x
  - Result: **VALID** (At exact cap limit)
