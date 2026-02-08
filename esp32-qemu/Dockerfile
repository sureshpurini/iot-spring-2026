# ESP32 Development Environment with QEMU
# For IoT Course - Spring 2026

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    git \
    wget \
    flex \
    bison \
    gperf \
    python3 \
    python3-pip \
    python3-venv \
    cmake \
    ninja-build \
    ccache \
    libffi-dev \
    libssl-dev \
    dfu-util \
    libusb-1.0-0 \
    libncurses5-dev \
    libncursesw5-dev \
    && rm -rf /var/lib/apt/lists/*

# Install QEMU dependencies
RUN apt-get update && apt-get install -y \
    libglib2.0-dev \
    libpixman-1-dev \
    libgcrypt20-dev \
    zlib1g-dev \
    libslirp-dev \
    && rm -rf /var/lib/apt/lists/*

# Set up ESP-IDF
ENV IDF_PATH=/opt/esp-idf
ENV IDF_TOOLS_PATH=/opt/esp-idf-tools

# Clone ESP-IDF v5.1 (stable release with good QEMU support)
RUN git clone --recursive --depth 1 --branch v5.1.2 \
    https://github.com/espressif/esp-idf.git ${IDF_PATH}

# Install ESP-IDF tools
RUN ${IDF_PATH}/install.sh esp32

# Clone and build Espressif QEMU
WORKDIR /opt
RUN git clone --depth 1 https://github.com/espressif/qemu.git esp-qemu

WORKDIR /opt/esp-qemu
# Install Python dependencies required by QEMU build system
RUN pip3 install tomli

RUN ./configure \
    --target-list=xtensa-softmmu \
    --enable-slirp \
    --disable-werror \
    --prefix=/opt/qemu-esp32 \
    && make -j$(nproc) \
    && make install

# Add QEMU to PATH
ENV PATH="/opt/qemu-esp32/bin:${PATH}"

# Create workspace directory
WORKDIR /workspace

# Create entrypoint script
RUN echo '#!/bin/bash\n\
source /opt/esp-idf/export.sh\n\
exec "$@"' > /entrypoint.sh && chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
