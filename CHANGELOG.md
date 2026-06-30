# Changelog

All notable changes to this library are documented in this file. The format is
based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this
project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2026-06-30

### Added

- `getPowerStatus()` to read the POWER_STATUS register (0x3F), exposing the advertised Type-C current (Rp level) for sources that don't negotiate PD.

## [1.0.0] - 2026-06-25

### Added

- Initial release: multiplatform driver for the TI TPS26750 USB Type-C Power Delivery controller over I2C (Arduino, ESP32, STM32, RP2040), built around a per-instance I2C bus handle, with CI and release workflows and example sketches.
