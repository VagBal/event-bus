# ==============
# Stage 1: Build & Test
# ==============
FROM ubuntu:22.04 AS build

# Default build is release
ARG BUILD_TYPE=Release
ARG ENABLE_GPROF=OFF

# No interactive promts during package install
ENV DEBIAN_FRONTEND=noninteractive

# Install GCC, CMake, Make, Git, GoogleTest, lcov for coverage
RUN apt-get update && apt-get install -y \
    g++ cmake make git libgtest-dev lcov && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Build GoogleTest (Ubuntu only gives the src code)
RUN cd /usr/src/googletest && \
    cmake . && make && \
    cp lib/*.a /usr/lib

WORKDIR /app

# Copy only build config first bc of caching
COPY CMakeLists.txt .

# Now copy the rest of the sources
COPY src/ src/
COPY include/ include/
COPY tests/ tests/

# Configure project with coverage flags for Debug builds
RUN if [ "${BUILD_TYPE}" = "Debug" ]; then \
        cmake -B build -S . \
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DENABLE_GPROF=${ENABLE_GPROF} \
            -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 --coverage -fprofile-arcs -ftest-coverage" \
            -DCMAKE_EXE_LINKER_FLAGS_DEBUG="--coverage"; \
    else \
        cmake -B build -S . \
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DENABLE_GPROF=${ENABLE_GPROF} \
            -DCMAKE_CXX_FLAGS_RELEASE="-Os -DNDEBUG" \
            -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-Os -g -DNDEBUG"; \
    fi

# Build and run unittests
RUN cmake --build build --config ${BUILD_TYPE} && \
    ctest --test-dir build --output-on-failure -V

# Generate coverage report for Debug builds
RUN if [ "${BUILD_TYPE}" = "Debug" ]; then \
        cd build && \
        lcov --capture --directory . --output-file coverage.info && \
        lcov --remove coverage.info '/usr/*' '*/tests/*' '*/_deps/*' --output-file coverage_filtered.info && \
        lcov --list coverage_filtered.info && \
        echo "========================================" && \
        echo "COVERAGE SUMMARY" && \
        echo "========================================" && \
        lcov --summary coverage_filtered.info && \
        COVERAGE_PERCENT=$(lcov --summary coverage_filtered.info 2>&1 | grep "lines" \
            | grep -oP '\d+\.\d+(?=%)' | head -1) && \
        echo "Coverage: ${COVERAGE_PERCENT}%" && \
        if [ -n "${COVERAGE_PERCENT}" ]; then \
            if awk -v cov="${COVERAGE_PERCENT}" 'BEGIN {exit !(cov >= 95.0)}'; then \
                echo "✓ Coverage target met: ${COVERAGE_PERCENT}% >= 95%"; \
            else \
                echo "✗ Coverage target NOT met: ${COVERAGE_PERCENT}% < 95%"; \
                echo "Coverage report saved to build/coverage_filtered.info"; \
            fi; \
        fi && \
        genhtml coverage_filtered.info --output-directory coverage_html && \
        echo "HTML coverage report generated in build/coverage_html"; \
    fi

# ==============
# Stage 2: Runtime
# ==============
FROM debian:bookworm-slim

ARG WITH_VALGRIND=0
ARG WITH_GRAPHVIZ=0
ARG WITH_GPROF=0
ENV DEBIAN_FRONTEND=noninteractive

# Update and install security patches
RUN apt-get update && \
    apt-get upgrade -y && \
    if [ "$WITH_VALGRIND" -eq "1" ]; then \
        apt-get install -y valgrind; \
    fi && \
    if [ "$WITH_GRAPHVIZ" -eq "1" ]; then \
        apt-get install -y python3 python3-venv graphviz; \
        python3 -m venv /opt/gprof2dot-venv; \
        /opt/gprof2dot-venv/bin/pip install --no-cache-dir gprof2dot; \
        ln -sf /opt/gprof2dot-venv/bin/gprof2dot /usr/local/bin/gprof2dot; \
    fi && \
    if [ "$WITH_GPROF" -eq "1" ]; then \
        apt-get install -y binutils; \
    fi && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Add a non-root user for sec
RUN useradd -m runner_user

# Copy just the build binary from Stage 1
COPY --from=build /app/build/event-bus .

# Copy coverage reports if they exist (for Debug builds)
RUN --mount=type=bind,from=build,source=/app/build,target=/tmp/build \
    if [ -f /tmp/build/coverage_filtered.info ]; then \
        cp /tmp/build/coverage_filtered.info ./coverage_filtered.info; \
    fi && \
    if [ -d /tmp/build/coverage_html ]; then \
        cp -r /tmp/build/coverage_html ./coverage_html; \
    fi

USER runner_user

# Default command
CMD ["./event-bus"]