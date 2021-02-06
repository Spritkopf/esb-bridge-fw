# esb-bridge

This firmware provides access to the proprietary RF protocol which I'm using for my homebrew smart home applications. The protocol is based on the Enhanced Shockburst (ESB) protocol by Nordic Semiconductor. The esb-bridge firmware allows a host PC to interact with the ESB-based devices.

The firmware implements a virtual com port with a simple binary command protocol. ESB data can be sent to any Pipeline address in atomic send-reply transactions (called transfer).
Also, a listener address can be set so the device can act as a "central" device (similar to BLE) and send a USB packet to the host on incoming ESB messages

The firmware is targeted for the NRF52840 chip and was tested on a Particle Xenon board.

## WORK IN PROGRESS
This is work in progress and not considered functional, as long as this notice exists

## How to build
Just clone the project and run `make`. A `gcc-arm-none-eabi` toolchain is needed (`arm-none-eabi-gcc` is expected in PATH)

## Limitations
* ESB connection parameters are fixed to suit my application
* The ESB implemention of the Nordic NRF5 SDK for the integrated radio is used and is therefore not suitable for the standalone NRF24L01 devices

## Lincense
If not mentioned otherwise in individual source files, the MIT license (see LINCESE file) is applicable