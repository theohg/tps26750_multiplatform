/**
 * @file source_capabilities.ino
 * @brief TPS26750 source-capability enumeration example.
 *
 * Reads the Power Delivery source capabilities advertised by the attached
 * USB-C supply and prints each Fixed/PPS/AVS power profile to Serial.
 *
 * @note Enumerating the capability register reads 53 bytes in a single I2C
 *       transaction. Classic AVR Arduino cores (e.g. Nano) cap the Wire buffer
 *       at 32 bytes; use ESP32, STM32, or RP2040 for full enumeration.
 */

#include <Wire.h>
#include <tps26750.h>

#define TPS26750_ADDR   0x21   // 7-bit I2C address (Address Index #2)
#define MAX_CAPS        13     // TPS26750 advertises up to 13 PDOs (SPR + EPR)

#if (defined(PICO_BOARD) || defined(PICO_RP2040) || defined(PICO_SDK_VERSION_MAJOR)) && !defined(ARDUINO)
#include "hardware/gpio.h"
void initBus() {
    i2c_init(i2c0, 400000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
}
TPS26750 pd(i2c0, TPS26750_ADDR);
#else
void initBus() {
    Wire.begin();
}
TPS26750 pd(&Wire, TPS26750_ADDR);
#endif

void setup() {
    Serial.begin(115200);
    initBus();

    Serial.println("TPS26750 source-capability reader.");
    if (!pd.init()) {
        Serial.println("TPS26750 not found. Check wiring and I2C address.");
    }
    Serial.println();
}

void loop() {
    TPS26750_SourceCapability caps[MAX_CAPS];
    uint8_t count = pd.getSourceCapabilities(caps, MAX_CAPS);

    Serial.print("Source offers ");
    Serial.print(count);
    Serial.println(" power profile(s):");

    for (uint8_t i = 0; i < count; i++) {
        Serial.print("  [");
        Serial.print(i);
        Serial.print("] ");

        if (caps[i].is_pps || caps[i].is_avs) {
            Serial.print(caps[i].is_pps ? "PPS " : "AVS ");
            Serial.print(caps[i].min_voltage_mv);
            Serial.print("-");
            Serial.print(caps[i].voltage_mv);
            Serial.print(" mV @ ");
            Serial.print(caps[i].max_current_ma);
            Serial.println(" mA");
        } else {
            Serial.print("Fixed ");
            Serial.print(caps[i].voltage_mv);
            Serial.print(" mV @ ");
            Serial.print(caps[i].max_current_ma);
            Serial.println(" mA");
        }
    }

    if (count == 0) {
        Serial.println("  (none - attach a USB-C PD source)");
    }

    Serial.println("---");
    delay(3000);
}
