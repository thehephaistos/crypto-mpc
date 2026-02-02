# Use Ubuntu 22.04 as base
FROM ubuntu:22.04

# Prevent interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsodium-dev \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy project files
COPY . .

# Create build directory and build
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make

# Default command: run all tests
CMD ["bash", "-c", "cd build && ./simple_test && ./comprehensive_test && ./secure_memory_test && ./mpc_foundation_test"]
