/**
 * @file pd_status_monitor.ino
 * @brief TPS26750 USB-PD status and active-contract monitoring example.
 *
 * Reads the controller mode, Type-C connection status, and the active Power
 * Delivery contract (voltage/current), printing a periodic summary to Serial.
 */

#include <Wire.h>
#include <tps26750.h>

#define TPS26750_ADDR   0x21   // 7-bit I2C address (Address Index #2)

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

    Serial.println("Initializing TPS26750 USB-PD controller...");
    if (!pd.init()) {
        Serial.println("TPS26750 not found. Check wiring and I2C address.");
    } else {
        Serial.println("TPS26750 ready.\n");
    }
}

void loop() {
    char mode[5];
    if (pd.getMode(mode)) {
        Serial.print("Mode: ");
        Serial.println(mode);
    }

    uint8_t status[5] = {0};
    if (pd.getStatus(status)) {
        Serial.print("Plug present: ");
        Serial.println((status[0] & TPS26750_STATUS_PLUG_PRESENT) ? "yes" : "no");

        Serial.print("Port role:    ");
        Serial.println((status[0] & TPS26750_STATUS_PORT_ROLE) ? "Source" : "Sink");

        Serial.print("Orientation:  ");
        Serial.println((status[0] & TPS26750_STATUS_ORIENTATION) ? "CC2 (flipped)" : "CC1");
    }

    uint32_t voltage_mv = 0;
    uint32_t current_ma = 0;
    if (pd.getActiveContract(voltage_mv, current_ma)) {
        Serial.print("Active contract: ");
        Serial.print(voltage_mv);
        Serial.print(" mV @ ");
        Serial.print(current_ma);
        Serial.println(" mA");
    } else {
        Serial.println("No active PD contract.");
    }

    Serial.println("---");
    delay(2000);
}
