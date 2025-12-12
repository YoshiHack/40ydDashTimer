Dec, 11, 2025 approx 9hr of work time
Now i worked on the cad file
im currently working on the casing for the laser modules, this is the first revision:<img width="1076" height="868" alt="Screenshot 2025-12-11 211436" src="https://github.com/user-attachments/assets/c1e1dd43-e132-4b1a-94a9-4273a450aff4" />
 this includes 1 laser hole, 2 holes for Led(s), a shelf for the ESP, a tripod mount, and cable holes

Today im working on the BOM
Today i worked on the Bill Of materials:image_2025-12-09_183159802

I made sure to include the Price, Quantity, Name, Description, And Purpose are all listed
added all of the materials required for the build you can view the BOM at BOM.csv
Today im working on the BOM
Today i worked on the Bill Of materials: <img width="1755" height="679" alt="Screenshot 2025-12-09 183141" src="https://github.com/user-attachments/assets/ef9bcfc4-8333-4b9d-a3ec-a828e8305398" />

I made sure to include the Price, Quantity, Name, Description, And Purpose are all listed
added all of the materials required for the build you can view the BOM at BOM.csv

Dec, 5, 2025 
Approx 1hr of total work time

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

Added basic code skeleton
We added these code strips, all code can be found at https://github.com/YoshiHack/40ydDashTimer
<img width="1747" height="689" alt="Screenshot 2025-12-05 192930" src="https://github.com/user-attachments/assets/27887378-32c6-4550-a80c-93f8206daadd" />

ESP-NOW Skeleton created for all three stations:
Finish Gate: This gate will be for stopping the timer and the new code is at Firmware/FinishGate/finish-gate.ino
Start Gate: This gate will be for starting the timer and the code is at Firmware/StartGate/start-gate.ino
The Base gate will be for controlling all of the modules and doing all of the processing the code is at Firmware/BaseStation/base-station.ino
