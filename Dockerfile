# Dockerfile for building an image used to build the firmware
LABEL Description="Image for building and debugging the firmware"

FROM alpine:3.15

RUN wget -qO- https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 | tar -xj -C /opt

ENV PATH "/opt/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"
