# README.md

## Overview

This is a sample program meant to illustrate the pattern of [reader-writer lock](https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock).
Most demonstrations of the reader-writer lock start some readers and writers and let them sleep for some fixed amount of time to simulate
the time during which they are reading or writing. This doesn't really allow one to effectively test their hypotheses and understand the pattern. To overcome this
limitation, this program works a bit differently. You control when to start a reader or writer and when to exit a reader or writer.
In this way you basically control the reader-writer lock. This is done using following commands (key presses):

- `r` will start a reader
- `w` will start a writer
- `x` will exit a reader
- `q` will exit a writer
- any other key press will exit the program

As example, to verify that no reader can read while a writer is writing, I will press following sequence of keys:

- `w` to start a writer and verify it can write
- `r` to start a reader and verify it cannot read
- `q` to exit the writer and verify the reader can now read
- `x` to exit the reader

Using above as guideline, verify following assertions:

- Assertion 1: No writer can start writing while a reader is reading. To verify this first start a reader (r) and then try to start a writer (w).
- Assertion 2: No writer can start writing while another writer is writing. To verify this first start a writer (w) and then try to start another writer (w).
- Assertion 3: No reader can start reading while a writer is writing. To verify this first start a writer (w) and then try to start another reader (r).
- Assertion 4: Multiple readers can read concurrently at the same time provided no writer is writing. To verify this first start a reader (r) and then try to start another reader (r).

In other words an X-lock will block. The table below shows two threads and the kind of lock they have acquired (or are attempting to acquire) on a shared mutex.
S stands for shared lock and X stands for exclusive lock.

| Thread 1 | Thread 2 | Blocks |
| --- | --- | --- |
| S | X | Yes |
| X | X | Yes |
| X | S | Yes |
| S | S | No |

## Build

```
./compile.sh
```

## Run

```
./main
```

## Understanding the Internals

The most important piece of code to understand in this program is this:

```
cv.wait(lock, [this] { return is_finished; });
```

What does it do? let's break it down into step-by-step:

1. `[this] { return is_finished; }` defines a lambda function in C++. The arguments inside`[...]` become part of the closure variables.
2. The lambda function is evaluated.
3. If the lambda function returns `true`, the thread will block until it has acquired the `lock`.
4. If the lambda function returns `false`, the thread will release the `lock` (only if it already has it), and will enter sleep state until its woken up by another thread via a call to `notify` or `notify_one`.
5. That's it. When the thread wakes up, all the steps starting from 1 will be repeated again.

The point to note here is that in step 3, it is entirely possible that the lambda function returns true, the thread blocks until it has acquired the lock
but by the time it has acquired the lock and ready to execute the next piece of code, the condition in the lambda function might no longer be true.
Because of this, in some places I have seen a `while` loop being used to recheck if the condition is still true (e.g. refer the section titled
Monitor usage on [this](https://en.wikipedia.org/wiki/Monitor_(synchronization)) page; I do not do this in my code btw):

```
std::unique_lock<std::mutex> lock {mx}; // acquire the lock
while (!condition) {                    // if condition is already true we don't need to enter the loop
    cv.wait(lock, [this] { return condition; });
}
```

To finish off, lets also cover what `notify` and `notify_one` do. When a thread calls `notify`, it will wake up all other threads waiting
on the CV (condition variable) whereas `notify_one` will wake up only one thread (the thread to wake up is picked at random amongst the pool of waiting
threads). waiting = sleeping.