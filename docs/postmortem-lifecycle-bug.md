# Postmortem — the mutation/lifecycle bug (post-jam, 31 Aug 2026)

## What I observed

I had the reset function (`ResetRound`) within a loop that was going through every single Agent (`for (Agent& agent : agents)` in `main.cpp`, the win branch of the click check).

`ResetRound` clears `agents` and then pushes the new round's agents back into it.

## Why it seemed fine

This is fine initially because the memory block allocated to the vector still exists after `clear()` (capacity is kept, only the size goes to 0), and the loop's iterator could still find real agents — the new round's agents — in the same block.

## Why it is a bug

A `std::vector` is one contiguous block, not one element at a time. When the agent list becomes too big for that block, `push_back` allocates a new block, moves every element across, and frees the old one. The loop is still holding an iterator into the old block, so from that point it is reading freed memory.

`clear()` invalidates every iterator and reference into the vector, so the loop's `++it` / `it != end()` after `ResetRound` is undefined behaviour. It *may* crash; it may also silently keep running over stale or reused memory and behave wrongly. A crash is the visible case, not the only one.

Side effect even in the "working" rounds: the mouse is still pressed for the rest of the frame, so the leftover iterations can judge a brand-new agent from the next round.

## When it fires

Trigger: the first win where `currentRound + 5 > agents.capacity()`.

Measured on my Mac (native build, printf before/after the `ResetRound` call in the win branch):

```
round 2  before: cap=8  data=0xbb6fa9980
round 2  after:  cap=8  data=0xbb6fa9980
round 3  before: cap=8  data=0xbb6fa9980
round 3  after:  cap=8  data=0xbb6fa9980
round 4  before: cap=8  data=0xb580f0780
round 4  after:  cap=16 data=0xb5822a700   <- block moved
round 5..7:      cap=16, address stable
```

Capacity 8 is reached when the title screen builds round 1 (6 agents, growth 1→2→4→8). Rounds 2 and 3 need 7 and 8 agents and fit. Round 4 needs 9 > 8, so `push_back` reallocates to 16 and frees the old block while the loop is still iterating it.

AddressSanitizer (`clang++ -fsanitize=address -g`) confirms it at round 4:

```
==6657==ERROR: AddressSanitizer: heap-use-after-free on address 0x6130000db734
READ of size 4 at 0x6130000db734 thread T0
    #0 main main.cpp:145                      <- switch (agent.AgentState), next loop iteration

0x6130000db734 is located 116 bytes inside of 384-byte region
freed by thread T0 here:
    ...
    #6 std::vector<Agent>::__emplace_back_slow_path   <- the reallocation
    #8 std::vector<Agent>::push_back
    #9 ResetRound(...) main.cpp:58
    #10 main main.cpp:201                     <- the win branch, inside the agent loop

previously allocated by thread T0 here:
    ...
    #9 ResetRound(...) main.cpp:58
    #10 main main.cpp:256                     <- title screen, first round build (capacity 8)

SUMMARY: AddressSanitizer: heap-use-after-free main.cpp:145 in main
```

Reading the report: `Agent` is 48 bytes, so the 384-byte region is the capacity-8 block, and offset 116 = 2 × 48 + 20 is `AgentState` of agent index 2 — the mimic was at index 1, I clicked it, and the loop carried on to index 2 of a block that had just been freed.

Why it never crashed without ASan: the allocator keeps a freed small block mapped and mostly intact, and `Agent` holds no pointers, so the stale iterations read (and wrote!) old bytes and nothing faulted. Writes into freed memory can corrupt allocator bookkeeping and crash somewhere unrelated later; the web build uses a different allocator, so the same code could behave differently there.

Regression check for the fix: the ASan build must run clean past round 4 (win at least 4 rounds in a row).

## The rule

TODO — one sentence I would apply anywhere, not just here.

## Fix chosen

Do like game lost: set a Won game state inside the loop and call `ResetRound` outside of the loop. Not `break` — that only holds while someone remembers why it is there, and it leaves the click check inside the agent loop (see the single-click resolution design).
