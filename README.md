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