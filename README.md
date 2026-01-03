# TNFS client C library

## TNFS – The Trivial Network File System

**TNFS** is a lightweight network filesystem protocol designed for simplicity
and portability. It is particularly suitable for 8-bit and 16-bit systems,
embedded devices, and retro computers, while still being usable on modern
operating systems.

Compared to FTP, TNFS behaves much more like a real filesystem.
Compared to NFS or SMB, it is intentionally minimal and easy to implement.
TNFS can operate over both **UDP and TCP**, making it usable even with very
limited network stacks.

### More about the TNFS protocol

For the full protocol specification and reference implementation, see the
official TNFS server repository:

- https://github.com/spectrumero/tnfsd
- Protocol documentation:
  https://github.com/spectrumero/tnfsd/blob/master/tnfs-protocol.md

## Why this TNFS client C library?

When discovering the TNFS protocol (for example through the Fujinet project),
there were very few TNFS clients available for desktop systems, and none that
were clean, portable, and written in C.

This project was created to:

- Provide a simple, readable TNFS client implementation in C
- Run on Linux and Windows
- Be easily portable to embedded systems
- Serve as a reference implementation for other TNFS projects

The library is intentionally structured so that:
- The TNFS protocol logic is platform-independent
- The networking layer is isolated and replaceable

This makes it suitable not only for PC testing, but also for use in embedded or
retro-computing projects (for example: an Atari ST ACSI → TNFS bridge).

## Project structure

- tnfs.c – TNFS protocol implementation (platform independent)
- netw.c – POSIX networking backend (Linux / Unix)
- netw_win32.c – Windows networking backend (Winsock)
- main.c – Demo / test program
- tnfs.h / netw.h – Public headers

## Supported platforms

- Linux (GCC, POSIX sockets)
- Windows (MinGW + Winsock)

Designed for easy porting to embedded and retro systems.

## Building

### Linux

```bash
./build.sh
```

Binary will be placed in the `build/` directory.

### Windows (MinGW)

```bat
build_debug.bat
```

Produces `build\tnfs_test.exe`.

## Test server

Download TNFS server binaries from:
https://github.com/spectrumero/tnfsd/tree/master/bin

Example:

```bash
./tnfsd ~/public
```

## Example usage

See `main.c` for a complete example covering:

- mount / umount
- directory listing
- file read/write
- stat, rename, unlink

## Notes

To port this library to another platform:
- Replace netw.c
- Keep tnfs.c unchanged
