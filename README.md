# Bluetooth LE CO2 Monitor

A wireless indoor air-quality monitor built from two Nordic nRF5340 development kits, Zephyr RTOS, and a Sensirion SCD41 CO2 sensor. It measures CO2 every minute, sends the readings over Bluetooth Low Energy, and turns them into something you can act on: live graphs, trends, and plain-language ventilation advice.

## Goal

CO2 is a good proxy for how stale the air in a room is. High levels cause drowsiness and poor concentration, but you can't see or smell them. This project aims to be a genuinely useful air-quality tool rather than a bare sensor demo, answering the questions that matter in practice:

- How good is the air right now?
- Is it getting better or worse, and how fast?
- Should I open a window, and how soon?

## Overview

The system has three parts:

| Part | Role |
|------|------|
| **Monitoring Device** (nRF5340 DK #1) | Reads the SCD41 over I2C every minute and sends each reading over BLE as a GATT notification. Takes an immediate reading when the Gateway asks. |
| **Gateway Device** (nRF5340 DK #2) | BLE central. Receives readings, computes rolling averages, rate of change, air quality level and time-to-next-level, and sends the results over serial. |
| **PC Dashboard** | Reads the Gateway's serial output, shows a live graph and status with ventilation advice, and logs every sample to CSV. |

```
 SCD41 --I2C--> Monitoring Device --BLE (GATT notify)--> Gateway Device --USB serial--> PC Dashboard
```

## Features

- Periodic CO2 measurements (every minute).
- BLE link using the standard Environmental Sensing Service and CO2 Concentration characteristic, with read and notify support.
- On-demand CO2 readings (GATT read triggers a fresh measurement), mainly for testing.
- Automatic reconnection if the BLE link drops, with stale-data detection.
- Gateway-side trend analysis: 5- and 15-minute averages, rate of change (ppm/min), and trend direction.
- Four air quality levels with hysteresis, plus an estimate of the time until the next level is reached.
- PC dashboard with live graph, colour-coded status, ventilation advice, and daily CSV logs (planned, see Dashboard TODO).

## Specification

| Item | Value |
|------|-------|
| Platform | Nordic nRF5340 DK (x2), Zephyr RTOS |
| Sensor | Sensirion SCD41, CO2 only, I2C address `0x62` |
| Sensor range / accuracy | 400 to 5000 ppm, roughly ±(40 ppm + 5% of reading) |
| Sample interval | 60 seconds |
| BLE roles | Monitoring Device is peripheral and GATT server; Gateway is central and GATT client |
| GATT service | Environmental Sensing Service (`0x181A`) |
| GATT characteristic | CO2 Concentration (`0x2B8C`), read + notify, CCCD for enabling notifications |
| Payload | Unsigned 16-bit little-endian value in ppm |
| Stale-data timeout | 150 seconds without a notification |
| Gateway sample buffer | 60 samples (one hour) |
| Serial link | USB virtual COM port, 115200 baud, 8N1, ASCII lines |
| PC dashboard | Python 3 (`pyserial` + plotting library) |

## Hardware

- 2 x Nordic nRF5340 Development Kits
- 1 x Sensirion SCD41 CO2 sensor (connected to the Monitoring Device over I2C)
- USB cable from the Gateway Device to the PC

## Technologies

- C
- Zephyr RTOS
- Bluetooth Low Energy (BLE) and GATT
- I2C
- Serial communication
- Python 3 (dashboard)

## Building and flashing

Build each firmware app for the nRF5340 application core and flash it to its own kit. Replace the paths with your app directories:

```
west build -b nrf5340dk/nrf5340/cpuapp <path-to-monitor-app> --pristine
west flash

west build -b nrf5340dk/nrf5340/cpuapp <path-to-gateway-app> --pristine
west flash
```

Use `west flash --dev-id <serial-number>` when both kits are plugged in, so each image goes to the right board.

## Dashboard TODO

The PC dashboard is not built yet. Planned work:

- [ ] Read and parse the Gateway's serial output (`pyserial`), ignoring malformed lines
- [ ] Live graph of CO2 over time with selectable 1 / 8 / 24 hour window
- [ ] Overlay 5- and 15-minute averages and show air quality level bands
- [ ] Status panel with current reading, level, trend, rate of change, and ventilation advice
- [ ] Connection and stale-data indicator
- [ ] Daily CSV logging with PC timestamps
- [ ] Auto-reconnect when the serial port disappears
- [ ] Show gaps in the graph instead of interpolating

## Scope

In scope: the Monitoring Device and Gateway firmware, the BLE link, Gateway trend analysis, the serial protocol, and the PC dashboard.

Out of scope: a mobile app, enclosure or PCB design(TBD), and any cloud backend. History buffering, runtime configuration over BLE, and OTA updates are deliberately left out to keep the design simple and robust.

## Documentation

See [CO2_Software_Design.pdf](CO2_Software_Design.pdf) for the software architecture, interfaces, and full design specification.

## Author

Oscar Ashburn
