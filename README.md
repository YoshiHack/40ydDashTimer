# 40-Yard Dash Wireless Timing System
A fully wireless, laser-based 40-yard dash timer built using ESP32 microcontrollers, ESP-NOW low-latency communication, and 3D-printed gate hardware. Designed for high-accuracy sprint timing with millisecond precision and clean field deployment.

---

## Project Overview
This project creates a professional-grade timing system for 40-yard dash performance testing.  
It uses two wireless timing gates (start and finish) and a base station to display times and log results.

Each gate transmits beam-break events using ESP-NOW, allowing extremely low-latency communication without Wi-Fi networks or physical cables.  
Laser modules provide tight, reliable beams to ensure accurate timing.

---


## Features
- Fully wireless start and finish gates  
- ESP-NOW communication (no router required)  
- Laser break-beam detection  
- Millisecond-level accuracy using ESP32 hardware timers  
- Custom 3D-printed gate housings  
- OLED display at base station  
- Audio + LED indicators for start/finish confirmation  
- Safe Li-ion power system with USB-C charging  

---

## System Architecture

### Start Gate Node
- Detects first beam break  
- Sends event packet over ESP-NOW  
- Battery-powered  
- LED indicators for alignment and status  

### Finish Gate Node
- Detects finish beam break  
- Sends event packet  
- Battery-powered  
- Identical housing to start gate  

### Base Station
- Receives timing packets from both gates  
- Computes sprint time  
- Displays results on OLED  
- Buzzer + LEDs for feedback  
- Can log data to laptop or custom app  

---

## Hardware Components

### Core Electronics
- 3× ESP32-WROOM DevKit V1 (Start, Finish, Base)

### Laser Timing System
- 2× 5V Laser Diode Modules  
- 2× Laser Receiver Modules  
- 4× Mini Ball-Head Mounts  
- 2× Tripods or custom 3D-printed stands  

### Indicators and User Feedback
- 1× 5V Active Piezo Buzzer  
- High-brightness green and red LEDs (2–4 total)  
- Resistors (220Ω recommended)

### Displays
- 1× 1.3" I2C OLED (128×64) for base station  

---

## Power System (Recommended Best Setup)
Each wireless gate uses:

- 1× Protected 18650 Li-ion battery  
- 1× TP4056 USB-C Li-ion charger module (with protection)  
- 1× DC-DC Boost Converter (3.7V → 5V)  
- 1× Panel-mount power switch  
- (Optional) MAX1704x fuel gauge for battery percentage reporting

Base Station can run off:
- Laptop USB  
or  
- The same 18650 + TP4056 + Boost system as the gates  

---

## Communication (ESP-NOW)
ESP-NOW is used for fast and stable wireless communication:

- Very low latency  
- Peer-to-peer  
- No Wi-Fi network needed  
- Reliable outdoors  

Start and Finish nodes send event packets to the Base Station, which timestamps them using the ESP32's internal timer.

---

## 3D Printing
Custom gate housings include:
- Mounts for ESP32 board  
- Battery compartment  
- Alignment mounts for laser and receiver  
- Windows for LEDs  
- Power switch and USB-C port access  

Recommended filament: **PETG** for outdoor durability.

---

## SIM_MODE Quick Test (No Hardware Needed)

All firmware supports `SIM_MODE` so the logic can be tested before parts arrive.

### Base Station
Open Serial Monitor @ **115200 baud**.

Keys:
- `h` = help
- `r` = reaction start (READY / SET / random delay / GO)
- `s` = simulate START trigger
- `f` = simulate FINISH trigger
- `p` = print CSV header
- `l` = toggle packet logging
- `x` = reset to IDLE

Expected:
- After `r`, wait for `GO!`, then press `s`, wait, press `f`
- You should see:
  - human-readable time output
  - a `CSV,...` line (paste into Google Sheets)
  - a `LOG,...` line (if logging is on)

### Start Gate / Finish Gate
Type `b` to simulate a beam break event (prints debug output).

---

## Development Status
- ✔ Repository created  
- ✔ Part selection in progress  
- ✔ Power architecture finalized  
- ☐ 3D models in progress  
- ☐ ESP-NOW firmware skeleton  
- ☐ Prototype gate builds  
- ☐ Outdoor testing  

---

## Screenshots
(Place images inside the `/hardware` or `/media` folder and embed them here.)

---

## Acknowledgments
Designed and developed by **Jackson Hughes** as a custom engineering and sprint performance tracking system.  
Built for accuracy, reliability, and expandability (multiple splits, leaderboards, mobile app integration, etc.).
