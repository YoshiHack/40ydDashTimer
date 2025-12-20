# Firmware

This project uses three ESP32 nodes:

- **StartGate**: detects start beam break and sends an ESP-NOW trigger packet
- **FinishGate**: detects finish beam break and sends an ESP-NOW trigger packet
- **BaseStation**: receives packets, runs reaction-mode start logic, computes elapsed time, and prints CSV output

## Simulation
Each sketch supports `SIM_MODE` for development without hardware.

## Shared Protocol
`shared/protocol.h` defines the packet format used between all devices.
