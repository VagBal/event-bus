# Event-Bus System for simulated sensors

A thread-safe, event-driven sensor simulation system built in C++17, a publish-subscribe architecture for handling concurrent sensor data streams.

Simulates multiple sensor types (CO gas, temperature, pressure) publishing readings to a central EventBus. Events are dispatched asynchronously to registered consumers on a dedicated worker thread. All sensors run concurrently with configurable intervals.

## Building and Running

### Prerequisites

- Docker (for containerized builds)
- Make (for build automation)
- C++17 compatible compiler (when building locally)
- CMake 3.14+ (when building locally)

### Quick Start

Build and run the release version:
```bash
make release
make run-release
```

For development with debug symbols:
```bash
make debug
make run-debug
```

### Local Build

If you prefer building without Docker:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
./event-bus
```

## Testing

The project includes comprehensive unit and integration tests covering all components. Tests are built automatically with the main binary.

### Running Tests

```bash
# Build and run tests inside Docker
docker build -f .devcontainer/Dockerfile -t event-bus:test .
docker run --rm event-bus:test

# Or build locally
cd build && ctest --output-on-failure
```

### Test Coverage

```bash
make coverage
# View report: .tool_result/coverage/coverage_html/index.html
```

## Profiling and Analysis

Run all analysis tools:
```bash
make all-tools
```

Individual tools:
- `make run-callgrind` - CPU profiling (output: `.tool_result/callgrind/`)
- `make run-gprof` - Function profiling (output: `.tool_result/gprof/gprof.txt`)
- `make run-memcheck` - Memory leak detection (output: `.tool_result/memcheck/memcheck.log`)
- `make run-helgrind` - Thread safety analysis (output: `.tool_result/helgrind/helgrind.log`)

## Documentation

Generate API documentation:
```bash
doxygen
```
Output: `docs/html/index.html` with full API reference, class diagrams, and call graphs.