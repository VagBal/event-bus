# ==============
# Stage 1: Build & Test
# ==============
FROM ubuntu:22.04 AS build

# Default build is release
ARG BUILD_TYPE=Release

# No interactive promts during package install
ENV DEBIAN_FRONTEND=noninteractive

# Install GCC, CMake, Make, Git, GoogleTest
RUN apt-get update && apt-get install -y \
    g++ cmake make git libgtest-dev && \
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
#COPY external/ external/

# Configure project
RUN cmake -B build -Os -S . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

# Build and run unittests
RUN cmake --build build --config ${BUILD_TYPE} && \
    ctest --test-dir build --output-on-failure -V

# ==============
# Stage 2: Runtime
# ==============
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Update and install security patches
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Add a non-root user for sec
RUN useradd -m runner_user
USER runner_user

# Copy just the build binary from Stage 1
COPY --from=build /app/build/event-bus .

# Default command
CMD ["./event-bus"]