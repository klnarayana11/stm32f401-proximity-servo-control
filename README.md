# stm32f401-ultrasonic-servo-rtos

> **Real-Time Ultrasonic Distance Measurement and Servo Motor Control System**  
> Built on STM32F401RBT6 using FreeRTOS, HC-SR04 Ultrasonic Sensor, Servo Motor, and 16x2 LCD Display.

---

## 📌 Project Overview

This project implements a real-time embedded system that continuously measures distance using an HC-SR04 ultrasonic sensor, classifies the measurement into three proximity zones, and actuates a servo motor to a corresponding angle. All system logic runs concurrently under FreeRTOS with dedicated tasks for sensing, actuation, and display. Distance and zone information is shown live on a 16x2 HD44780 LCD.

---

## 🛠️ Hardware Components

| Component | Description |
|---|---|
| **STM32F401RBT6** | ARM Cortex-M4 microcontroller @ 16MHz (HSI) |
| **HC-SR04** | Ultrasonic distance sensor (2cm – 400cm range) |
| **Servo Motor** | PWM-controlled, 0°–180° rotation |
| **16x2 LCD (HD44780)** | 4-bit parallel interface monochrome display |
| **External 5V Supply** | Powers servo and sensor independently from MCU |

---

## ✨ Features

- Real-time distance measurement using **Timer Input Capture** — hardware-level timing, no software jitter
- Three-zone proximity classification — **Near / Mid / Far**
- Servo motor actuation to **0° / 90° / 180°** based on detected zone
- Live distance and zone display on **16x2 LCD** updating every 500ms
- FreeRTOS multi-tasking with **task notifications** and **queues**
- ISR to task signalling — ISR stays minimal, heavy work done in task context
- Clean modular architecture — separate driver files for ultrasonic, servo, and LCD
- Timer overflow / wrap-around handling for accurate pulse-width measurement

---

## 📐 System Architecture

```
HC-SR04 Sensor
    │
    ├── TRIG ◄── PA0  (GPIO Output)             10µs trigger pulse
    └── ECHO ──► PA1  (TIM2_CH2 Input Capture)  echo pulse width measurement

STM32F401RBT6 @ 16MHz (HSI)
    │
    ├── TIM2 CH2  → Input Capture  → ISR Callback → vSonarTask
    ├── TIM3 CH1  → PWM Output     → Servo Signal (PA6)
    └── GPIOB     → 4-bit LCD      → PB0(RS) PB1(EN) PB4-PB7(D4-D7)

FreeRTOS Tasks
    │
    ├── vSonarTask  (Priority 3) — Trigger sensor, read distance, classify zone
    ├── vServoTask  (Priority 2) — Receive zone, move servo to mapped angle
    └── vLcdTask    (Priority 1) — Display distance and zone on LCD
```

---

## 📦 Distance Zones

| Zone | Distance Range | Servo Angle |
|---|---|---|
| **NEAR** | 0 cm – 10 cm | 0° |
| **MID** | 10 cm – 30 cm | 90° |
| **FAR** | > 30 cm | 180° |

---

## 🗂️ File Structure

```
Core/
├── Inc/
│   ├── main.h
│   ├── freertos_app.h       — Task handles, queue handles, config macros
│   ├── ultrasonic.h         — HC-SR04 driver interface & zone definitions
│   ├── servo.h              — Servo PWM driver interface
│   └── lcd.h                — 16x2 LCD 4-bit driver interface
│
└── Src/
    ├── main.c               — Hardware init, clock config, FreeRTOS start
    ├── freertos_app.c       — Task implementations, queue management
    ├── ultrasonic.c         — Input capture callback, distance calc, zone classify
    ├── servo.c              — PWM CCR angle control
    └── lcd.c                — HD44780 4-bit driver, init sequence, cursor, write
```

---

## ⚙️ Timer Configuration

### TIM2 — Ultrasonic Input Capture (Echo Pin)

| Parameter | Value |
|---|---|
| System Clock | 16 MHz (HSI) |
| Prescaler | 15 |
| Timer Clock | 1 MHz (1 tick = 1 µs) |
| Counter Period | 65535 (16-bit max) |
| Channel | CH2 — Input Capture on PA1 |
| Capture Polarity | Rising first, toggled to Falling in callback |
| NVIC | TIM2 Global Interrupt Enabled |

```
Prescaler calculation:
16MHz / (15 + 1) = 1MHz → 1 tick = 1µs ✓
Max measurable pulse = 65535µs ≈ 65ms
HC-SR04 max echo     = 38ms   → safely within range ✓
```

### TIM3 — Servo PWM

| Parameter | Value |
|---|---|
| System Clock | 16 MHz (HSI) |
| Prescaler | 319 |
| Timer Clock | 50 KHz |
| Counter Period (ARR) | 999 |
| Output Frequency | 50 Hz (20ms period) |
| Channel | CH1 — PWM Output on PA6 |
| CCR for 0° | 50 (1ms pulse) |
| CCR for 90° | 75 (1.5ms pulse) |
| CCR for 180° | 100 (2ms pulse) |

```
Prescaler calculation:
16MHz / (319 + 1) = 50KHz

Frequency:
50KHz / (999 + 1) = 50Hz → 20ms period ✓

CCR to pulse width:
CCR=50  → 50  / 50KHz = 1.0ms → 0°
CCR=75  → 75  / 50KHz = 1.5ms → 90°
CCR=100 → 100 / 50KHz = 2.0ms → 180°
```

---

## 🔌 Pin Mapping

| STM32 Pin | Connected To | Mode | Function |
|---|---|---|---|
| PA0 | HC-SR04 TRIG | GPIO Output Push-Pull | Send 10µs trigger pulse |
| PA1 | HC-SR04 ECHO | TIM2_CH2 Input Capture | Measure echo pulse width |
| PA6 | Servo Signal | TIM3_CH1 PWM Output | Control servo angle position |
| PB0 | LCD RS | GPIO Output Push-Pull | Command / Data select |
| PB1 | LCD EN | GPIO Output Push-Pull | Clock data into LCD |
| PB4 | LCD D4 | GPIO Output Push-Pull | Data bit 4 |
| PB5 | LCD D5 | GPIO Output Push-Pull | Data bit 5 |
| PB6 | LCD D6 | GPIO Output Push-Pull | Data bit 6 |
| PB7 | LCD D7 | GPIO Output Push-Pull | Data bit 7 (MSB) |

> ⚠️ **Voltage Warning:** HC-SR04 ECHO pin outputs 5V logic. STM32F401 GPIO pins are **not 5V tolerant**. Use a voltage divider (1KΩ series + 2KΩ to GND) on the ECHO line before connecting to PA1 to bring the signal down to ~3.3V.

---

## 🔄 FreeRTOS Task Flow

```
Every 60ms:

vSonarTask (Priority 3)
    │
    ├─ Ultrasonic_Trigger()               — 10µs pulse on PA0
    ├─ ulTaskNotifyTake(50ms timeout)     — BLOCKS, zero CPU used
    │       ▲
    │       └── ISR: HAL_TIM_IC_CaptureCallback()
    │               ├── Rising edge  → save t1, flip polarity to falling
    │               └── Falling edge → save t2, compute distance,
    │                                  classify zone,
    │                                  vTaskNotifyGiveFromISR()
    │                                  portYIELD_FROM_ISR()
    │
    ├─ Ultrasonic_GetZone()               — read classified result
    ├─ xQueueOverwrite(xZoneQueue)        — send zone to ServoTask
    └─ vTaskDelay(60ms)                   — wait before next trigger
                │
                ▼
        vServoTask (Priority 2)
                │
                ├─ xQueueReceive(xZoneQueue) — BLOCKS until zone arrives
                └─ Servo_SetZoneAngle()      — write CCR to TIM3

        vLcdTask (Priority 1)
                │
                ├─ Ultrasonic_GetDistance()  — read latest distance
                ├─ Ultrasonic_GetZone()      — read latest zone
                ├─ LCD_SetCursor(0,0)        — Row 0: distance
                ├─ LCD_SetCursor(1,0)        — Row 1: zone + angle
                └─ vTaskDelay(500ms)         — refresh every 500ms
```

---

## 📺 LCD Display Format

```
┌────────────────┐
│Dist: 18.7cm    │   ← Row 0: live distance reading
│MID  Ang: 90 dg │   ← Row 1: zone name + servo angle
└────────────────┘

Zone examples:
┌────────────────┐    ┌────────────────┐    ┌────────────────┐
│Dist:  5.2cm    │    │Dist: 18.7cm    │    │Dist: 45.0cm    │
│NEAR Ang:  0 dg │    │MID  Ang: 90 dg │    │FAR  Ang:180 dg │
└────────────────┘    └────────────────┘    └────────────────┘
     NEAR zone              MID zone              FAR zone
```

---

## 🚀 Getting Started

### Prerequisites

- STM32CubeIDE v1.12 or later
- ST-Link V2 programmer / debugger
- arm-none-eabi-gcc toolchain (bundled with STM32CubeIDE)

### Build & Flash

```bash
# 1. Clone the repository
git clone https://github.com/your-username/stm32f401-ultrasonic-servo-rtos.git

# 2. Open in STM32CubeIDE
#    File → Open Projects from File System → select cloned folder

# 3. Build the project
#    Project → Build Project  (Ctrl + B)

# 4. Flash and run
#    Run → Debug   (F11)        — flash + debug mode
#    Run → Run     (Ctrl+F11)   — flash + run mode
```

### Wiring Quick Reference

```
HC-SR04:
  VCC  → 5V
  GND  → GND (common)
  TRIG → PA0
  ECHO → [1KΩ] → PA1 → [2KΩ] → GND    ← voltage divider required!

Servo Motor:
  VCC    → External 5V supply  (NOT STM32 3.3V pin!)
  GND    → GND (common ground with STM32)
  Signal → PA6

16x2 LCD (HD44780):
  VDD    → 5V
  VSS    → GND
  V0     → Potentiometer wiper  (contrast adjustment)
  RS     → PB0
  RW     → GND  (always write mode)
  EN     → PB1
  D4     → PB4
  D5     → PB5
  D6     → PB6
  D7     → PB7
  A(BL+) → 3.3V via 100Ω resistor
  K(BL-) → GND
```

---

## 📐 Distance Formula

```
Distance (cm) = Echo Pulse Width (µs) / 58

Derivation:
  Speed of sound = 343 m/s = 0.0343 cm/µs
  Sound travels TO object and BACK (round trip)
  One-way distance = (pulse_µs × 0.0343) / 2
                   = pulse_µs / 58.3
                   ≈ pulse_µs / 58
```

---

## 💾 Memory Usage (SRAM — 64KB available)

| Resource | Size |
|---|---|
| vSonarTask stack | 256 words = 1024 bytes |
| vServoTask stack | 256 words = 1024 bytes |
| vLcdTask stack | 256 words = 1024 bytes |
| FreeRTOS TCBs (3 tasks) | ~300 bytes |
| xZoneQueue | ~50 bytes |
| Global variables | ~100 bytes |
| **Total estimated usage** | **~3.5 KB** |
| **Free SRAM remaining** | **~60.5 KB** |

---

## ⚠️ Known Limitations

- HC-SR04 minimum reliable range is **2cm** — readings below this are inaccurate
- HC-SR04 maximum range is **400cm** — no echo beyond this, handled by 50ms timeout
- Servo must be powered from **external 5V supply** — connecting to STM32 3.3V will cause MCU reset under motor load
- LCD contrast must be adjusted via **potentiometer on V0 pin** at first power-up
- System clock uses internal **HSI oscillator at 16MHz** — no external crystal required

---

## 🧠 Key Concepts Demonstrated

- FreeRTOS task creation, priorities, and preemptive scheduling
- Inter-task communication using **queues** (`xQueueOverwrite`, `xQueueReceive`)
- ISR to task signalling using `vTaskNotifyGiveFromISR` and `ulTaskNotifyTake`
- `portYIELD_FROM_ISR` for immediate context switch after ISR notification
- Hardware **Timer Input Capture** for microsecond-accurate pulse measurement
- **Timer overflow wrap-around** handling in pulse width calculation
- **PWM generation** for servo motor position control via CCR register
- HD44780 **16x2 LCD driver** in 4-bit mode with proper datasheet init sequence
- Modular embedded C driver architecture with clean public header interfaces

---

## 👨‍💻 Author

**Kodi Balaji**  
Embedded Systems Developer  
STM32 | FreeRTOS | ARM Cortex-M4

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
