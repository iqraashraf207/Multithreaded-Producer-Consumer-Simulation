# Multi-Threaded Producer-Consumer Simulation

A concurrent systems simulation in C implementing the classic producer-consumer problem using PThreads, semaphores, and mutex synchronization.

---

## What it does

Multiple producer threads generate random items and insert them into a shared circular buffer. Multiple consumer threads remove and process those items. Semaphores control buffer capacity and mutex locks prevent race conditions during concurrent access.

---

## Synchronization Mechanism

| Primitive | Role |
|---|---|
| `emptySem` | Tracks available empty slots in the buffer |
| `fullSem` | Tracks available filled slots in the buffer |
| `bufMutex` | Ensures mutual exclusion during buffer access |

---

## Build & Run

    make
    ./projectcode

On startup the program prompts for:

- Buffer size
- Number of producers
- Number of consumers
- Items per thread

---

## Sample Output

    [PRODUCER 0] generating item:  42  (1 of 5)
    [PRODUCER 0] inserted  item:  42
      Buffer: [ 42][   ][   ]  (in = 1, out = 0)
    --------------------------------------------------
    [CONSUMER 0] trying to consume  (1 of 5)
    [CONSUMER 0] removed   item:  42
      Buffer: [   ][   ][   ]  (in = 1, out = 1)
    --------------------------------------------------

---

## Project Structure

    producer-consumer/
    ├── projectcode.c
    ├── Makefile
    └── README.md

---

## Tech Stack

`C` `PThreads` `POSIX Semaphores` `Mutex` `Makefile`
