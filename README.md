*This project has been created as part of the 42 curriculum by ainradan.*

# Description

**codexion** is a concurrent programming exercise in C that simulates coders racing against burnout while competing for scarce USB dongles. It implements POSIX threads, mutexes, condition variables, and sophisticated scheduling algorithms (FIFO and EDF) to orchestrate resource sharing, prevent deadlocks, and ensure fair access — all while keeping your coders productive before the deadline strikes.

Each coder is a thread that must acquire two dongles (left and right) to compile quantum code. After compiling, they debug and refactor before attempting to compile again. The simulation ends when either every coder has completed the required number of compiles, or one coder burns out from waiting too long.

---

# Instructions

## Compilation

```bash
make
```

The project compiles with `-Wall -Wextra -Werror -pthread`.

## Makefile Rules

| Command | Description |
|---------|-------------|
| `make` | Compile the codexion executable |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and executable |
| `make re` | Full rebuild |

## Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

### Arguments

| Argument | Description |
|----------|-------------|
| `number_of_coders` | Number of coders (and dongles) |
| `time_to_burnout` | Max ms since last compile start before burnout |
| `time_to_compile` | Duration of compiling phase (ms) |
| `time_to_debug` | Duration of debugging phase (ms) |
| `time_to_refactor` | Duration of refactoring phase (ms) |
| `number_of_compiles_required` | Compiles needed per coder to stop |
| `dongle_cooldown` | Cooldown after releasing a dongle (ms) |
| `scheduler` | `fifo` or `edf` |

### Examples

```bash
# FIFO scheduler, 3 coders, 4000ms burnout, 200ms per phase, 5 compiles, 100ms cooldown
./codexion 3 4000 200 200 200 5 100 fifo

# EDF scheduler, 5 coders, tight deadline
./codexion 5 800 100 100 100 3 50 edf
```

---

# Blocking Cases Handled

## Deadlock Prevention
Deadlock is prevented by enforcing a **strict total ordering** on dongle acquisition: each coder always picks the lower-ID dongle first, then the higher-ID one. This breaks the circular-wait condition (one of Coffman's four conditions), ensuring no cyclic dependency can form even when all coders attempt to acquire resources simultaneously. A special case is handled when there is only one coder: the same dongle is used for both hands without attempting to lock it twice.

## Starvation Prevention
Under FIFO scheduling, a **global ticket counter** guarantees strict arrival ordering across all requests, preventing any coder from being indefinitely bypassed. Under EDF, the priority queue always serves the coder with the earliest burnout deadline first. Liveness is guaranteed because every compile updates the requester's deadline, and the monitor ensures no coder is left waiting past their threshold.

## Cooldown Handling
After a dongle is released, it enters a cooldown state. Threads waiting for that dongle use `pthread_cond_timedwait` to sleep until `free_since + dongle_cooldown`. This prevents busy-waiting and ensures precise re-availability without wasting CPU cycles.

## Precise Burnout Detection
A dedicated **monitor thread** tracks each coder's last compile start time. It sleeps via `pthread_cond_timedwait` until the earliest upcoming burnout deadline. If the deadline passes without a compile start signal (`ETIMEDOUT`), the monitor logs the burnout within 10ms and broadcasts a stop signal to all threads and dongles.

## Log Serialization
All state-change logs pass through a single `pthread_mutex_t` (`print_lock`). This guarantees that no two messages interleave on the same line, preserving the required output format even under heavy concurrent logging.

---

# Thread Synchronization Mechanisms

## Primitives Used

### `pthread_mutex_t`
- **`print_lock`**: Serializes all `printf` calls to prevent interleaved log lines.
- **`stop_lock`**: Protects the global stop flag read by `is_stopped()` and set by `request_stop()`.
- **`count_lock`**: Guards the per-coder compile counters during `register_compile()`.
- **`state_lock`**: Synchronizes access to `last_compile_start` timestamps and the monitor's condition variable.
- **`ticket_lock`**: Ensures atomic increment of the global FIFO ticket counter.
- **Per-dongle `lock`**: Each dongle has its own mutex protecting its availability state, cooldown timestamp, and request heap.

### `pthread_cond_t`
- **`state_cond`**: Used by the monitor thread to wait for compile-start events or until the next burnout deadline expires.
- **Per-dongle `cond`**: Threads waiting to acquire a dongle sleep here. It is broadcast when:
  - A dongle is released (availability change)
  - A cooldown expires (timed wake-up)
  - The simulation stops (emergency wake-up)

## Custom Event Implementation
The dongle request queue is a **min-heap binary priority queue**. For FIFO, priority is a monotonic global ticket; for EDF, priority is the coder's burnout deadline (`last_compile_start + time_to_burnout`). The heap is protected by the per-dongle mutex. A coder can only take a dongle if they are at the front of its heap, the dongle is available, and its cooldown has elapsed.

## Race Condition Prevention
- **Double-dongle acquisition**: A coder inserts a heap-allocated `t_request` before waiting. If the simulation stops before acquisition, the request is explicitly removed from the heap and freed, preventing phantom queue entries.
- **Spurious wakeups**: The `while` loop around `pthread_cond_wait`/`pthread_cond_timedwait` re-checks all conditions (stopped, available, cooldown elapsed, front-of-heap) before proceeding.
- **Monitor precision**: The monitor holds `state_lock` while computing deadlines and waiting, ensuring it never reads a stale timestamp while a coder is mid-update.

---

# Resources

## Documentation & References
- **The Linux Programming Interface** — Michael Kerrisk (Chapters 29–31: POSIX Threads, Mutexes, Condition Variables)
- **POSIX Threads Programming** — randu Tutorial: https://randu.org/tutorials/threads/
- **pthread_cond_timedwait(3)** — Linux man pages
- **Dining Philosophers Problem** — Classic concurrency analogy; our solution uses resource ordering (low-ID first) to prevent deadlock

## AI Usage
AI tools were used during this project for:
- **Brainstorming deadlock prevention strategies** (discussing Coffman's conditions and resource ordering)
- **Reviewing condition variable usage** (ensuring correct `pthread_cond_wait` loop patterns and spurious wakeup handling)
- **Debugging burnout timing issues** (identifying that `pthread_cond_timedwait` was needed for cooldown expiration instead of unconditional `pthread_cond_wait`)
