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
- 3× ESP32-WROOM DevKit V1 (Start, Finish, Base)<img width="894" height="613" alt="image" src="https://github.com/user-attachments/assets/0cc6e4b1-039e-400b-a1e7-0956fb012e26" />


### Laser Timing System
- 2× 5V Laser Diode Modules  <img width="800" height="600" alt="image" src="https://github.com/user-attachments/assets/1ac48543-4447-4301-b667-ae62e9334253" />
- 2× Laser Receiver Modules  
- 4× Mini Ball-Head Mounts  
- 2× Tripods or custom 3D-printed stands  

### Indicators and User Feedback
- 1× 5V Active Piezo Buzzer  
- High-brightness green and red LEDs (2–4 total)  
- Resistors (220Ω recommended)

### Displays
- 1× 1.3" I2C OLED (128×64) for base station  <img width="750" height="412" alt="image" src="https://github.com/user-attachments/assets/8acf896d-b961-4460-9b7d-fae80f2dc704" />

---

## Power System (Recommended Best Setup)
Each wireless gate uses:

1. Amazon Battery pack ( I dont want to risk a fire )<img width="894" height="965" alt="image" src="https://github.com/user-attachments/assets/631309ba-68cb-4c59-af69-7c4f58c2e818" />
2. A USB-A to USB-B Cable

Base Station can run off:
- Laptop USB  
or  
- The same system as the gates  

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
<img width="1076" height="868" alt="Screenshot 2025-12-11 211436" src="https://github.com/user-attachments/assets/7bb221db-198a-490e-8f30-427e9d9ac9b4" />

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
- Repository created  
- Part selection finished  
- Power architecture finalized  
- 3D models finished  
- ESP-NOW firmware completed  
- No prototype gate builds preformed yet  
- No Outdoor testing  

---

## Screenshots
<img width="506" height="389" alt="Screenshot 2025-12-11 200626" src="https://github.com/user-attachments/assets/481d02ce-d1fc-4692-96c1-10a2ce77cbcc" />
<img width="656" height="450" alt="Screenshot 2025-12-11 204724" src="https://github.com/user-attachments/assets/eb2fc81e-aa13-49ef-8a62-183b7c50935f" />
<img width="1168" height="856" alt="Screenshot 2025-12-05 234738" src="https://github.com/user-attachments/assets/9a83712f-048a-4192-acdc-7c5c9fe2e63e" />

---

## Acknowledgments
Designed and developed by **Jackson Hughes** as a custom engineering and sprint performance tracking system.  
Built for accuracy, reliability, and expandability (multiple splits, leaderboards, mobile app integration, etc.).
