/**
 * @file tps26750_platform_i2c.h
 * @brief Platform-agnostic I2C function declarations for the TPS26750 library.
 *
 * Provides a common I2C API used by the TPS26750 driver class. The
 * implementation is selected at compile time (Arduino Wire, STM32 HAL, or Pico SDK).
 *
 * These helpers expose only raw bus transfers (probe, buffered write, and
 * write-then-read). The chip-specific "Unique Address Interface" framing
 * (leading byte-count on reads and writes) is assembled by the driver class,
 * not here.
 *
 * @note Device addresses are always passed as 7-bit addresses.
 *       STM32 HAL shifts them internally; Arduino Wire and Pico SDK expect 7-bit.
 *
 * @copyright Copyright (c) 2026 Theo Heng
 * @license MIT License. See LICENSE file for details.
 */

#ifndef TPS26750_PLATFORM_I2C_H
#define TPS26750_PLATFORM_I2C_H

#include <stddef.h>
#include <stdint.h>

#include "tps26750_platform_config.h"

#ifdef TPS26750_PLATFORM_ARDUINO
    #include <Wire.h>
#endif

#if defined(PLATFORM_ARDUINO)
    typedef TwoWire* bus_handle_t;
#elif defined(PLATFORM_STM32)
    typedef I2C_HandleTypeDef* bus_handle_t;
#elif defined(PLATFORM_RP2040)
    typedef i2c_inst_t* bus_handle_t;
#else
    typedef void* bus_handle_t;
#endif

/* ──────────────────── Common I2C API ──────────────────── */

#define TPS26750_OK         0
#define TPS26750_ERR_I2C    1
#define TPS26750_ERR_HANDLE 2

/**
 * @brief Check whether a TPS26750 acknowledges its address on the bus.
 * @param bus Platform-specific I2C bus handle.
 * @param device_address 7-bit I2C address of the TPS26750.
 * @return true if the device responds, false otherwise.
 */
bool tps26750_i2c_probe(bus_handle_t bus, uint8_t device_address);

/**
 * @brief Write a buffer to the TPS26750 as a single I2C transaction (with STOP).
 * @param bus Platform-specific I2C bus handle.
 * @param device_address 7-bit I2C address of the TPS26750.
 * @param data Bytes to transmit (the driver places [reg][count][payload] here).
 * @param length Number of bytes in @p data.
 * @return true on success, false on failure.
 */
bool tps26750_i2c_write(bus_handle_t bus, uint8_t device_address, const uint8_t* data, size_t length);

/**
 * @brief Write a register pointer, then read back data with a repeated start.
 * @param bus Platform-specific I2C bus handle.
 * @param device_address 7-bit I2C address of the TPS26750.
 * @param reg Register offset written before the repeated-start read.
 * @param dest Destination buffer for the read bytes.
 * @param length Number of bytes to read into @p dest.
 * @return true on success, false on failure.
 */
bool tps26750_i2c_write_read(bus_handle_t bus, uint8_t device_address, uint8_t reg, uint8_t* dest, size_t length);

#endif /* TPS26750_PLATFORM_I2C_H */
