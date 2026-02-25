# RakNet

A cross-platform C++ networking library for games and real-time applications.

## About this fork

This is a modernized fork of the original [RakNet 4.081](https://github.com/facebookarchive/RakNet) by Oculus VR, Inc. The key changes in this fork are:

- **CMake modernization** — target-based CMake 3.14 build system with proper install/export support
- **Conventional directory layout** — headers in `include/RakNet/`, sources in `src/`, samples in `samples/`
- **Removed Visual Studio project files** — CMake generates IDE projects (Visual Studio, Xcode, etc.)
- **Single library target** — one `RakNet` target, switchable between static and shared via `BUILD_SHARED_LIBS`

## Features

- Reliable and unreliable UDP messaging with prioritization
- Remote procedure calls (RPC4)
- NAT traversal (punch-through, port forwarding via UPnP)
- Lobby, rooms, and matchmaking plugins
- Autopatcher for distributing game updates
- Voice chat (RakVoice)
- Replica Manager for object replication and serialization
- Secure connections with SLNet encryption
- ~60 included sample programs and a comprehensive test suite

## Project structure

```
include/RakNet/          Public headers
src/                     Library source files
samples/                 Sample programs (~60)
tests/                   Test suite (stress tests, benchmarks, verification programs)
extensions/              Optional extensions (SQLite, Autopatcher, etc.)
cmake/                   CMake packaging support
```

## Building

### Prerequisites

- CMake 3.14 or newer
- A C++11 compiler (GCC, Clang, MSVC, etc.)

### Configure and build

```bash
cmake -B build
cmake --build build
```

To install:

```bash
cmake --install build
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `BUILD_SHARED_LIBS` | `OFF` | Build shared library instead of static |
| `RAKNET_ENABLE_SAMPLES` | `ON` | Build the sample programs |
| `RAKNET_ENABLE_EXTENSIONS` | `ON` | Build dependent extensions |
| `RAKNET_ENABLE_TESTS` | `ON` | Build the test suite |

Example — build as a shared library without samples:

```bash
cmake -B build -DBUILD_SHARED_LIBS=ON -DRAKNET_ENABLE_SAMPLES=OFF
cmake --build build
```

## Usage

After installing, consume RakNet from your own CMake project:

```cmake
find_package(RakNet REQUIRED)
target_link_libraries(your_target PRIVATE RakNet::RakNet)
```

## Samples

The `samples/` directory contains ~60 programs demonstrating individual RakNet features such as chat, file transfer, NAT traversal, lobby systems, and more. Enable them with `-DRAKNET_ENABLE_SAMPLES=ON` (on by default).

## Tests

The `tests/` directory contains stress tests, benchmarks, and verification programs covering packet splitting, flow control, reliability, multi-threaded access, and more. Enable them with `-DRAKNET_ENABLE_TESTS=ON` (on by default).

## License

BSD 2-Clause License. See [LICENSE](LICENSE) and [PATENTS](PATENTS) for details.
