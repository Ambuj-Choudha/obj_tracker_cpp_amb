FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG ONNXRUNTIME_VERSION=1.20.1

# Install apt dependencies from the single source of truth. sudo is
# installed first so the postCreateCommand and shells can run privileged
# commands without rebuilding the image.
COPY .devcontainer/apt-packages.txt /tmp/apt-packages.txt
RUN apt-get update \
    && apt-get install -y --no-install-recommends sudo \
    && grep -vE '^\s*(#|$)' /tmp/apt-packages.txt \
       | xargs apt-get install -y --no-install-recommends \
    && rm -rf /var/lib/apt/lists/* /tmp/apt-packages.txt

# Default gcc/g++ to 14 so CMAKE_CXX_STANDARD 23 resolves cleanly.
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100

# ONNX Runtime: not in the Ubuntu archive, so pull the upstream release
# and expose a CMake config so `find_package(onnxruntime CONFIG REQUIRED)`
# resolves it.
RUN set -eux; \
    arch="$(uname -m)"; \
    case "$arch" in \
        x86_64) ort_arch=x64 ;; \
        aarch64) ort_arch=aarch64 ;; \
        *) echo "unsupported arch: $arch" >&2; exit 1 ;; \
    esac; \
    curl -fsSL -o /tmp/ort.tgz \
        "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-linux-${ort_arch}-${ONNXRUNTIME_VERSION}.tgz"; \
    mkdir -p /opt/onnxruntime; \
    tar -xzf /tmp/ort.tgz --strip-components=1 -C /opt/onnxruntime; \
    rm /tmp/ort.tgz; \
    ldconfig /opt/onnxruntime/lib

RUN mkdir -p /opt/onnxruntime/lib/cmake/onnxruntime && \
    cat > /opt/onnxruntime/lib/cmake/onnxruntime/onnxruntimeConfig.cmake <<'EOF'
set(onnxruntime_INCLUDE_DIR "/opt/onnxruntime/include")
set(onnxruntime_LIB_DIR "/opt/onnxruntime/lib")

add_library(onnxruntime::onnxruntime SHARED IMPORTED)
set_target_properties(onnxruntime::onnxruntime PROPERTIES
    IMPORTED_LOCATION "${onnxruntime_LIB_DIR}/libonnxruntime.so"
    INTERFACE_INCLUDE_DIRECTORIES "${onnxruntime_INCLUDE_DIR}")
EOF

ENV CMAKE_PREFIX_PATH=/opt/onnxruntime \
    LD_LIBRARY_PATH=/opt/onnxruntime/lib

# ubuntu:24.04 ships a non-root `ubuntu` user (uid 1000) — grant passwordless sudo.
RUN echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu \
    && chmod 0440 /etc/sudoers.d/ubuntu

USER ubuntu
WORKDIR /workspaces/obj_tracker_cpp_amb
