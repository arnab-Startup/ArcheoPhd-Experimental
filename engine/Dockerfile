# ==============================================================================
# Stage 1: Build Stage (C++20, GCC, CMake, Zstandard)
# ==============================================================================
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    curl \
    ca-certificates \
    libzstd-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Download industry-standard single-header C++ libraries: nlohmann/json and cpp-httplib
RUN mkdir -p include src && \
    curl -sSL -o include/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp && \
    curl -sSL -o include/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.18.3/httplib.h

COPY CMakeLists.txt ./
COPY src/ ./src/

RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel $(nproc)

# ==============================================================================
# Stage 2: Minimal Native Runtime (<25 MB total container footprint)
# ==============================================================================
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libzstd1 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy stripped native binary from builder
COPY --from=builder /app/build/archaeophd_engine /app/archaeophd_engine

# Storage directories
RUN mkdir -p /data/chunks /data/lancedb /data/models /data/cache

ENV ARCHAEOPHD_DATA_DIR=/data
EXPOSE 8000

CMD ["/app/archaeophd_engine"]
