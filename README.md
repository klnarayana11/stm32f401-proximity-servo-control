# stm32f401-ultrasonic-servo-rtos

> **Real-Time Ultrasonic Distance Measurement and Servo Motor Control System**  
> Built on STM32F401RBT6 using FreeRTOS, HC-SR04 Ultrasonic Sensor, Servo Motor, and 16x2 LCD Display.

---

## 📌 Project Overview

This project implements a real-time embedded system that continuously measures distance using an HC-SR04 ultrasonic sensor, classifies the measurement into three proximity zones, and actuates a servo motor to a corresponding angle. All system logic runs concurrently under FreeRTOS with dedicated tasks for sensing, actuation, and display.

---

## 🛠️ Hardware Components

| Component | Description |
|---|---|
| **STM32F401RBT6** | ARM Cortex-M4 microcontroller @ 84MHz |
| **HC-SR04** | Ultrasonic distance sensor (2cm – 400cm) |
| **Servo Motor** | PWM-controlled, 0°–180° rotation |
| **16x2 LCD (HD44780)** | 4-bit parallel interface display |
| **External 5V Supply** | Powers servo and sensor independently |

---

## ✨ Features

- Real-time distance measurement using **Timer Input Capture** (hardware-level, no software jitter)
- Three-zone proximity classification — **Near / Mid / Far**
- Servo motor actuation to **0° / 90° / 180°** based on zone
- Live distance and zone display on **16x2 LCD**
- FreeRTOS multi-tasking with **task notifications** and **queues**
- Clean modular code — separate driver files for ultrasonic, servo, and LCD
- Timer overflow handling for accurate pulse-width measurement

---

## 📐 System Architecture

```
HC-SR04 Sensor
    │
    ├── TRIG ◄── PA0 (GPIO Output)          10µs trigger pulse
    └── ECHO ──► PA1 (TIM2_CH2 Input Capture)  echo pulse width

STM32F401RBT6
    │
    ├── TIM2 CH2  → Input Capture  → ISR Callback → vSonarTask
    ├── TIM1 CH1  → PWM Output     → Servo Signal (PA8)
    └── GPIOB     → 4-bit LCD      → PB0(RS) PB1(EN) PB4-PB7(D4-D7)

FreeRTOS Tasks
    │
    ├── vSonarTask  (Priority 3) → Trigger + Read distance
    ├── vServoTask  (Priority 2) → Move servo to zone angle
    └── vLcdTask    (Priority 1) → Display distance + zone
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
│   ├── ultrasonic.h         — HC-SR04 driver interface
│   ├── servo.h              — Servo PWM driver interface
│   └── lcd.h                — 16x2 LCD 4-bit driver interface
│
└── Src/
    ├── main.c               — Hardware init, clock config, RTOS start
    ├── freertos_app.c       — Task implementations, queue management
    ├── ultrasonic.c         — Input capture callback, distance calc, zone classify
    ├── servo.c              — PWM angle control
    └── lcd.c                — HD44780 4-bit driver, init sequence, cursor, write
```

---

## ⚙️ Timer Configuration

### TIM2 — Ultrasonic Input Capture (Echo Pin)

| Parameter | Value |
|---|---|
| Clock | 84 MHz |
| Prescaler | 83 |
| Timer Clock | 1 MHz (1 tick = 1 µs) |
| Counter Period | 65535 |
| Channel | CH2 — Input Capture on PA1 |
| NVIC | TIM2 Global Interrupt Enabled |

### TIM1 — Servo PWM

| Parameter | Value |
|---|---|
| Clock | 84 MHz |
| Prescaler | 839 |
| Timer Clock | 100 KHz |
| Counter Period | 1999 |
| Output Frequency | 50 Hz (20ms period) |
| Channel | CH1 — PWM Output on PA8 |

---

## 🔌 Pin Mapping

| STM32 Pin | Connected To | Mode | Function |
|---|---|---|---|
| PA0 | HC-SR04 TRIG | GPIO Output | Send 10µs trigger pulse |
| PA1 | HC-SR04 ECHO | TIM2_CH2 Input Capture | Measure echo pulse width |
| PA8 | Servo Signal | TIM1_CH1 PWM Output | Control servo angle |
| PB0 | LCD RS | GPIO Output | Command / Data select |
| PB1 | LCD EN | GPIO Output | Enable clock pulse |
| PB4 | LCD D4 | GPIO Output | Data bit 4 |
| PB5 | LCD D5 | GPIO Output | Data bit 5 |
| PB6 | LCD D6 | GPIO Output | Data bit 6 |
| PB7 | LCD D7 | GPIO Output | Data bit 7 |

> ⚠️ **Important:** HC-SR04 ECHO outputs 5V. Use a voltage divider (1KΩ + 2KΩ) on the ECHO line before connecting to PA1. STM32F401 GPIO is **not 5V tolerant**.

---

## 🔄 FreeRTOS Task Flow

```
vSonarTask (Priority 3)
    │
    ├─ Ultrasonic_Trigger()          — 10µs pulse on PA0
    ├─ ulTaskNotifyTake()            — BLOCKS until ISR signals
    ├─ Ultrasonic_GetZone()          — read classified zone
    └─ xQueueOverwrite(xZoneQueue)   — send zone to ServoTask
                                              │
                                     vServoTask (Priority 2)
                                              │
                                      xQueueReceive()  — BLOCKS until zone arrives
                                      Servo_SetZoneAngle()
                                              │
                                     vLcdTask (Priority 1)
                                              │
                                      Ultrasonic_GetDistance()
                                      Ultrasonic_GetZone()
                                      LCD_WriteString()     — updates every 500ms
```

---

## 📺 LCD Display Format

```
┌────────────────┐
│Dist: 18.7cm    │   ← Row 0: live distance
│MID  Ang: 90 dg │   ← Row 1: zone name + servo angle
└────────────────┘
```

---

## 🚀 Getting Started

### Prerequisites

- STM32CubeIDE v1.12 or later
- STM32CubeMX (for regenerating config if needed)
- ST-Link V2 programmer
- arm-none-eabi-gcc toolchain

### Build & Flash

```bash
# 1. Clone the repository
git clone https://github.com/your-username/stm32f401-ultrasonic-servo-rtos.git

# 2. Open in STM32CubeIDE
File → Open Projects from File System → select cloned folder

# 3. Build
Project → Build Project  (Ctrl + B)

# 4. Flash
Run → Debug  (F11)  or  Run → Run  (Ctrl + F11)
```

---

## 📐 Distance Formula

```
Distance (cm) = Echo Pulse Width (µs) / 58

Derivation:
  Speed of sound = 343 m/s = 0.0343 cm/µs
  Round trip → divide by 2
  Distance = (pulse_µs × 0.0343) / 2
           = pulse_µs / 58.3
           ≈ pulse_µs / 58
```

---

## ⚠️ Known Limitations

- HC-SR04 minimum range is **2cm** — readings below this are unreliable
- HC-SR04 maximum range is **400cm** — beyond this no echo returns (handled by 50ms timeout)
- Servo powered from external 5V — **never** connect servo VCC to STM32 3.3V pin
- LCD contrast must be tuned via potentiometer on V0 pin at first power-up

---

## 🧠 Concepts Demonstrated

- FreeRTOS task creation, priorities, and scheduling
- Inter-task communication using queues (`xQueueOverwrite`, `xQueueReceive`)
- ISR to task signalling using `vTaskNotifyGiveFromISR` and `ulTaskNotifyTake`
- Hardware Timer Input Capture for microsecond-accurate pulse measurement
- Timer overflow / wrap-around handling
- PWM generation for servo motor control
- HD44780 16x2 LCD driver in 4-bit mode
- Modular embedded C driver architecture

---

## 👨‍💻 Author

**Kodi Balaji**  
Embedded Systems Engineer  
STM32 | FreeRTOS | ARM Cortex-M

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
