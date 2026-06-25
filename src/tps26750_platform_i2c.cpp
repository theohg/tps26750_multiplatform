/**
 * @file tps26750_platform_i2c.cpp
 * @brief Platform-specific I2C implementations for the TPS26750 library.
 *
 * Arduino/ESP32 path uses Wire; STM32 path uses HAL_I2C_*; RP2040 uses the Pico SDK.
 *
 * @copyright Copyright (c) 2026 Theo Heng
 * @license MIT License. See LICENSE file for details.
 */

#include "tps26750_platform_i2c.h"

/**
 * @brief Check whether a TPS26750 acknowledges its address on the bus.
 * @param bus The bus handle to use.
 * @param device_address The I2C address of the TPS26750.
 */
bool tps26750_i2c_probe(bus_handle_t bus, uint8_t device_address)
{
    if (bus == NULL)
    {
        return false;
    }

#ifdef TPS26750_PLATFORM_ARDUINO
    bus->beginTransmission(device_address);
    return bus->endTransmission() == 0;
#elif defined(TPS26750_PLATFORM_STM32)
    return HAL_I2C_IsDeviceReady(bus, (uint16_t)(device_address << 1), 1, HAL_MAX_DELAY) == HAL_OK;
#elif defined(TPS26750_PLATFORM_RP2040)
    uint8_t reg = 0;
    return i2c_write_blocking(bus, device_address, &reg, 1, false) == 1;
#else
    (void)device_address;
    return false;
#endif
}

/**
 * @brief Write a buffer to the TPS26750 as a single I2C transaction (with STOP).
 * @param bus The bus handle to use.
 * @param device_address The I2C address of the TPS26750.
 * @param data The bytes to transmit.
 * @param length The number of bytes in data.
 */
bool tps26750_i2c_write(bus_handle_t bus, uint8_t device_address, const uint8_t* data, size_t length)
{
    if (bus == NULL || data == NULL || length == 0)
    {
        return false;
    }

#ifdef TPS26750_PLATFORM_ARDUINO
    bus->beginTransmission(device_address);
    bus->write(data, length);
    return bus->endTransmission() == 0;
#elif defined(TPS26750_PLATFORM_STM32)
    return HAL_I2C_Master_Transmit(bus, (uint16_t)(device_address << 1), (uint8_t*)data, (uint16_t)length, HAL_MAX_DELAY) == HAL_OK;
#elif defined(TPS26750_PLATFORM_RP2040)
    return i2c_write_blocking(bus, device_address, data, length, false) == (int)length;
#else
    (void)device_address;
    (void)data;
    (void)length;
    return false;
#endif
}

/**
 * @brief Write a register pointer, then read back data with a repeated start.
 * @param bus The bus handle to use.
 * @param device_address The I2C address of the TPS26750.
 * @param reg The register offset to write before the repeated-start read.
 * @param dest The destination buffer for the read bytes.
 * @param length The number of bytes to read into dest.
 */
bool tps26750_i2c_write_read(bus_handle_t bus, uint8_t device_address, uint8_t reg, uint8_t* dest, size_t length)
{
    if (bus == NULL || dest == NULL || length == 0)
    {
        return false;
    }

#ifdef TPS26750_PLATFORM_ARDUINO
    bus->beginTransmission(device_address);
    bus->write(reg);
    if (bus->endTransmission(false) != 0)
    {
        return false;
    }

    if (bus->requestFrom(device_address, static_cast<uint8_t>(length)) != length)
    {
        return false;
    }

    for (size_t index = 0; index < length; ++index)
    {
        if (!bus->available())
        {
            return false;
        }
        dest[index] = bus->read();
    }
    return true;
#elif defined(TPS26750_PLATFORM_STM32)
    return HAL_I2C_Mem_Read(bus, (uint16_t)(device_address << 1), reg, I2C_MEMADD_SIZE_8BIT, dest, (uint16_t)length, HAL_MAX_DELAY) == HAL_OK;
#elif defined(TPS26750_PLATFORM_RP2040)
    if (i2c_write_blocking(bus, device_address, &reg, 1, true) != 1)
    {
        return false;
    }
    return i2c_read_blocking(bus, device_address, dest, length, false) == (int)length;
#else
    (void)device_address;
    (void)reg;
    (void)dest;
    (void)length;
    return false;
#endif
}
