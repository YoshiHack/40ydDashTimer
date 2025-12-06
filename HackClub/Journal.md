Journal Entry – December 5, 2025

Today I started planning the 40-yard Dash Timer and created the GitHub repository: https://github.com/YoshiHack/40ydDashTimerimage_2025-12-05_190850729

I began selecting the core components and defining the system architecture.

Core Electronics: I'm deciding between using two or three ESP32 Dev Boards (ESP32-WROOM DevKit V1).
The start and finish gates will use ESP-NOW to transmit beam-break events with low latency.
Right now, timing data outputs to my laptop, but I may add an OLED screen to the base station.

Laser Timing Gates: Using two laser diode modules paired with laser receivers to get precise millisecond timing at both the start and finish lines.

Mounting System: Using four ball-head mounts and two tripods to align and stabilize each laser gate.

Indicators & User Feedback: Including a 5V active piezo buzzer for start/finish confirmation, and possibly a small OLED display to show run times.
Also planning to add high-brightness red/green LEDs for gate status indicators.

Power System
Using a protected 1S 18650 Li-ion setup with a TP4056 USB-C charging board and a 5V boost converter for reliable power at each wireless node.
(This provides safe charging and a stable power rail for the ESP32 and laser modules.)

ESP-NOW will be used for communication between the start gate, finish gate, and the base station.
