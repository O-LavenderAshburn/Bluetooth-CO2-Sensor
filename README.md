# Bluetooth LE CO2 Monitor

A CO2 monitoring system built using two Nordic nRF5340 development kits, Zephyr RTOS, and a Sensirion SCD41 sensor.

## Overview

The Monitoring Device reads CO2 measurements from the SCD41 sensor over I2C and transmits them over Bluetooth Low Energy (BLE).

The Gateway Device receives measurements via BLE and forwards them to a PC over serial.

## Hardware

- 2 × Nordic nRF5340 Development Kits
- 1 × Sensirion SCD41 CO2 Sensor

## Technologies

- C
- Zephyr RTOS
- Bluetooth Low Energy (BLE)
- GATT
- I2C
- Serial communication

## Features

- Periodic CO2 measurements (every minute).
- BLE communication between monitoring and gateway devices.
- GATT read and notification support.
- On-demand CO2 readings for testing.
- Serial output from the gateway to a PC.

## Documentation

See [CO2_Software_Design.pdf](CO2_Software_Design.pdf) for the software architecture, interfaces, and design specification.
