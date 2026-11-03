# Wiring Guide

Target board: **STM32 Nucleo-F401RE** (or any Nucleo with Arduino headers)

> **Note**: All signals on the Nucleo board operate at **3.3 V logic**. Do not connect 5 V signals directly.

---

## Potentiometer (ADC Input)

Connect a 10 kΩ potentiometer to the Arduino analog header:

| Potentiometer Pin | Nucleo Pin | Description |
|-------------------|------------|-------------|
| Left (CCW end) | GND | Ground reference |
| Middle (wiper) | A0 (PA0) | ADC1 Channel 0 input |
| Right (CW end) | 3V3 | 3.3 V supply |

The ADC reads 0–4095 (12-bit) corresponding to 0–3300 mV.

---

## Push Button

| Button Pin | Nucleo Pin | Description |
|------------|------------|-------------|
| Pin 1 | PC13 | B1 USER button (already on board) |
| Pin 2 | GND | Already on board |

The onboard B1 USER button on PC13 with internal pull-up is the default configuration. No external wiring required for the button.

If using an external button:

| External Button Pin | Nucleo Pin |
|--------------------|------------|
| One side | GND |
| Other side | Any GPIO configured as INPUT with pull-up (e.g. PB4) |

---

## Status LEDs

| LED Color | Nucleo Pin | Description |
|-----------|------------|-------------|
| Green (LD2) | PA5 | Already on board — healthy status |
| Red (external) | PB3 | Fault status — add 220Ω series resistor |

External red LED circuit:
```
PB3 ──[220Ω]──[LED(+)]──[LED(-)]── GND
```

---

## I2C Sensor (MPU-6050 or BME280)

Connect to the I2C1 bus on the Arduino I2C header:

| Sensor Pin | Nucleo Pin | Description |
|------------|------------|-------------|
| VCC | 3V3 | Power supply |
| GND | GND | Ground |
| SDA | D14 (PB9) | I2C1 data |
| SCL | D15 (PB8) | I2C1 clock |
| AD0 / SDO | GND | Set I2C address to 0x68 (MPU-6050) |

**Pull-up resistors**: The Nucleo I2C pins have no built-in pull-ups. Add 4.7 kΩ pull-up resistors from SDA and SCL to 3.3 V if the sensor breakout board does not include them.

### I2C Addresses

| Sensor | Default Address | Address with AD0/SDO HIGH |
|--------|-----------------|---------------------------|
| MPU-6050 | 0x68 | 0x69 |
| BME280 | 0x76 | 0x77 |

---

## USB Serial Connection

The Nucleo board's ST-LINK USB connector provides a virtual COM port at 115200 bps when plugged into a laptop. No additional wiring needed.

If using hardware UART instead:

| Signal | Nucleo Pin | Notes |
|--------|------------|-------|
| TX | D8 (PA9) / CN3 TX | USART1 TX |
| RX | D2 (PA10) / CN3 RX | USART1 RX |
| GND | GND | Common ground with host |

Use a 3.3 V USB-to-UART adapter (CP2102, CH340, or FT232). Do not use a 5 V adapter directly.

---

## Complete Wiring Summary

```
Nucleo-F401RE
┌─────────────────────────────────────────────────────┐
│ 3V3 ──────────────────────────────────────┐         │
│ GND ──────────────────────────────────────┤         │
│                                           │         │
│ A0 (PA0)  ◄── POT wiper                  │         │
│ PA5       ──► Green LED (LD2, on-board)   │         │
│ PB3       ──► [220Ω] ──► Red LED ──► GND │         │
│ PC13      ◄── B1 USER button (on-board)   │         │
│ PB8 (SCL) ◄──► I2C sensor SCL ◄──[4k7]──┤         │
│ PB9 (SDA) ◄──► I2C sensor SDA ◄──[4k7]──┘         │
│ USB CN1   ──── Laptop (115200 bps serial)           │
└─────────────────────────────────────────────────────┘
```
