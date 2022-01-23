# Dockerfile for building an image used to build the firmware
FROM alpine:3.15

LABEL Description="Image for building and debugging the firmware"

# Install ARM Embedded toolchain and Nordic SDK
RUN apk add make && \
    apk add gcc-arm-none-eabi && \
    apk add newlib-arm-none-eabi && \
    wget -qO- https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5_SDK_17.1.0_ddde560.zip |unzip -d /opt -

ENV SDK_ROOT "/opt/nRF5_SDK_17.1.0_ddde560"
ENV GNU_INSTALL_ROOT ""

# when running the container, mount the repository root to /workspace
WORKDIR /workspace