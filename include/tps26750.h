/**
 * @file tps26750.h
 * @brief Driver class for the TI TPS26750 USB Type-C Power Delivery controller.
 *
 * C++ driver for the TI TPS26750 USB-PD controller via I2C. Implements the
 * chip-specific "Unique Address Interface" protocol and supports Arduino/ESP32
 * (Wire), STM32 (HAL), and RP2040 (Pico SDK) platforms.
 *
 * Reference: TPS26750 Technical Reference Manual (SLVUCR7).
 *
 * @see https://www.ti.com/product/TPS26750
 * @copyright Copyright (c) 2026 Theo Heng
 * @license MIT License. See LICENSE file for details.
 */

#pragma once
#include "tps26750_platform_i2c.h"

#define TPS26750_LIB_VERSION          ("1.0.0")

// I2C Default Address Options (Decoded from ADCINx pins)
// #1: 0x20, #2: 0x21, #3: 0x22, #4: 0x23
#define TPS26750_I2C_ADDR_DEFAULT     0x21

// ============================================================================
// Bit Masks & Definitions
// ============================================================================

// --- STATUS Register (0x1A) Masks ---
#define TPS26750_STATUS_PLUG_PRESENT       (1 << 0)
#define TPS26750_STATUS_CONN_STATE_MASK    (0x0E)      // Bits 3-1
#define TPS26750_STATUS_CONN_NO_CONNECTION (0x00)
#define TPS26750_STATUS_CONN_AUDIO         (0x04)      // 010b << 1
#define TPS26750_STATUS_CONN_DEBUG         (0x06)      // 011b << 1
#define TPS26750_STATUS_CONN_PRESENT_NO_RA (0x0C)      // 110b << 1
#define TPS26750_STATUS_CONN_PRESENT_RA    (0x0E)      // 111b << 1
#define TPS26750_STATUS_ORIENTATION        (1 << 4)    // 0 = Upside-up (CC1), 1 = Upside-down (CC2)
#define TPS26750_STATUS_PORT_ROLE          (1 << 5)    // 0 = Sink, 1 = Source
#define TPS26750_STATUS_DATA_ROLE          (1 << 6)    // 0 = UFP, 1 = DFP
#define TPS26750_STATUS_VBUS_STATUS_MASK   (0x300000)  // Bits 21-20 (Byte 2)
#define TPS26750_STATUS_VBUS_SAFE0V        (0x00)
#define TPS26750_STATUS_VBUS_SAFE5V        (0x100000)
#define TPS26750_STATUS_VBUS_VALID         (0x200000)

// --- POWER_STATUS Register (0x3F) Masks ---
#define TPS26750_PWR_STATUS_CONNECTION      (1 << 0)    // 1 = power connection present
#define TPS26750_PWR_STATUS_SOURCE_SINK     (1 << 1)    // 0 = sinking, 1 = sourcing
#define TPS26750_PWR_STATUS_TYPEC_CURR_MASK (0x0C)      // Bits 3-2: advertised Type-C current
#define TPS26750_PWR_STATUS_TYPEC_USB_DEF   (0x00)      // 00b USB default (Rp-default)
#define TPS26750_PWR_STATUS_TYPEC_1_5A      (0x04)      // 01b 1.5A @ 5V (Rp-1.5A)
#define TPS26750_PWR_STATUS_TYPEC_3_0A      (0x08)      // 10b 3.0A @ 5V (Rp-3.0A)
#define TPS26750_PWR_STATUS_TYPEC_PD        (0x0C)      // 11b PD contract negotiated

// --- POWER_PATH_STATUS Register (0x26) Masks ---
#define TPS26750_PP_STATUS_PP_CABLE1_SW_MASK (0x03)
#define TPS26750_PP_STATUS_PP1_SWITCH_MASK   (0x1C0)         // Bits 8-6 (5V Path)
#define TPS26750_PP_STATUS_PP3_SWITCH_MASK   (0x7000)        // Bits 14-12 (HV/Ext Path)
#define TPS26750_PP_STATUS_PP1_OVERCURRENT   (1 << 28)
#define TPS26750_PP_STATUS_POWER_SOURCE_MASK (0xC000000000)  // Bits 39-38
#define TPS26750_PP_STATUS_SOURCE_VIN3V3     (0x4000000000)  // 01b
#define TPS26750_PP_STATUS_SOURCE_VBUS       (0x8000000000)  // 10b (Dead Battery)

// --- INT_EVENT1 Register (0x14) Masks ---
// Note: This register is 11 bytes wide. These are bit offsets from byte 0.
#define TPS26750_INT_PD_HARD_RESET          (1ULL << 1)
#define TPS26750_INT_PLUG_INSERT_REMOVAL    (1ULL << 3)
#define TPS26750_INT_POWER_SWAP_COMPLETE    (1ULL << 4)
#define TPS26750_INT_DATA_SWAP_COMPLETE     (1ULL << 5)
#define TPS26750_INT_NEW_CONTRACT_AS_SINK   (1ULL << 12)
#define TPS26750_INT_NEW_CONTRACT_AS_SOURCE (1ULL << 13)
#define TPS26750_INT_SOURCE_CAP_RX          (1ULL << 14)
#define TPS26750_INT_CMD1_COMPLETE          (1ULL << 30)
#define TPS26750_INT_ERROR_PROTOCOL         (1ULL << 38)
#define TPS26750_INT_READY_FOR_PATCH        (1ULL << 81)  // Byte 10, bit 1

// --- 4CC Commands (for CMD1 Register) ---
#define TPS26750_CMD_GAID      "Gaid"  // Warm Restart
#define TPS26750_CMD_GAID_COLD "GAID"  // Cold Reset
#define TPS26750_CMD_SWSK      "SWSk"  // Swap to Sink
#define TPS26750_CMD_SWSR      "SWSr"  // Swap to Source
#define TPS26750_CMD_GSRC      "GSrC"  // Get Source Caps (Used to re-negotiate Sink Contract)
#define TPS26750_CMD_GSKC      "GSkC"  // Get Sink Caps
#define TPS26750_CMD_ESRC      "ESrC"  // EPR Get Source Caps (Request EPR profiles 28V/36V/48V)
#define TPS26750_CMD_GPPI      "GPPI"  // Get Port Partner Information
#define TPS26750_CMD_MBRD      "MBRd"  // Message Buffer Read
#define TPS26750_CMD_PBME      "PBMe"  // Patch Bundle Mode Exit

// for sendGppiAndRead() / getManufacturerInfo() frame selection
enum tps26750_gppi_frame_enum {
  TPS26750_GPPI_FRAME_SOP           = 0,
  TPS26750_GPPI_FRAME_SOP_PRIME     = 1,
  TPS26750_GPPI_FRAME_SOP_DBL_PRIME = 2
};

// Represents one "line item" from the charger (e.g., "15V @ 3A")
struct TPS26750_SourceCapability {
    uint32_t voltage_mv;           // Fixed: Voltage. PPS/AVS: Max Voltage.
    uint32_t max_current_ma;       // Max Current (15-20V band for SPR AVS; overall for others)
    bool is_pps;                   // Programmable Power Supply (SPR)
    bool is_avs;                   // Adjustable Voltage Supply (SPR or EPR)
    // For PPS/AVS: voltage_mv is the MAX voltage, current is max current
    uint32_t min_voltage_mv;       // Only valid if is_pps or is_avs = true
    /// @brief SPR AVS only: max current for the 9-15V band (USB PD 3.2 §6.4.2).
    /// Zero for all other PDO types, including EPR AVS.
    uint32_t max_current_9_15_ma;
};

// ============================================================================
// Class Definition
// ============================================================================

class TPS26750
{
public:
    /**
     * @brief Construct a TPS26750 driver instance.
     * @param bus  Platform-specific I2C bus handle (`&Wire`, `&hi2c1`, `i2c0`, ...).
     * @param addr 7-bit I2C address (default 0x21 for Address Index #2).
     */
    TPS26750(bus_handle_t bus, uint8_t addr = TPS26750_I2C_ADDR_DEFAULT);

    /**
     * @brief Initialize the driver and verify device presence.
     * @return true if the device acknowledges and the MODE register reads back.
     */
    bool init();

    /**
     * @brief Probe the bus for the device.
     * @return true if the device acknowledges its address.
     */
    bool isConnected();

    /** @brief Get the configured 7-bit I2C address. */
    uint8_t getAddress();

    // ------------------------------------------------------------------------
    // Core Register Access (Implements Unique Address Protocol)
    // ------------------------------------------------------------------------

    /**
     * @brief Read a register using the specific TPS26750 protocol.
     * @details Protocol: Start -> Addr(Wr) -> Reg -> Sr -> Addr(Rd) -> ByteCount(N) -> Data... -> Stop.
     * The ByteCount is read from the device but stripped from the output buffer.
     *
     * @param reg Register offset.
     * @param dest Buffer to store read data.
     * @param len Number of bytes to read (must match register size).
     * @return true on success.
     */
    bool readRegister(uint8_t reg, uint8_t* dest, uint8_t len);

    /**
     * @brief Write a register using the specific TPS26750 protocol.
     * @details Protocol: Start -> Addr(Wr) -> Reg -> ByteCount(N) -> Data... -> Stop.
     * The ByteCount is automatically prepended.
     * @param reg Register offset.
     * @param src Data to write.
     * @param len Number of bytes to write.
     * @return true on success.
     */
    bool writeRegister(uint8_t reg, const uint8_t* src, uint8_t len);

    // ------------------------------------------------------------------------
    // High Level Functions
    // ------------------------------------------------------------------------

    /**
     * @brief Get the current Mode string (e.g., "APP ", "BOOT", "PTCH").
     * @param modeStr Buffer of at least 5 bytes (4 chars + null).
     */
    bool getMode(char* modeStr);

    /**
     * @brief Send a 4CC Command.
     * @param cmd 4-character string (e.g., "Gaid").
     */
    bool sendCommand(const char* cmd);

    /**
     * @brief Read the Interrupt Event Register.
     * @param events Buffer of 11 bytes to store the event flags.
     */
    bool readInterrupts(uint8_t* events);

    /**
     * @brief Clear specific interrupts.
     * @param mask Buffer of 11 bytes representing bits to clear.
     */
    bool clearInterrupts(const uint8_t* mask);

    /**
     * @brief Check if a specific interrupt bit is set in the provided buffer.
     * @param buffer 11-byte interrupt buffer.
     * @param bitIndex Bit index (0-87) to check.
     */
    bool isInterruptSet(const uint8_t* buffer, uint8_t bitIndex);

    /**
     * @brief Read the active contract Voltage and Current.
     * @details Correctly parses Fixed, PPS, and AVS contracts.
     * @param voltage_mv Output voltage in mV.
     * @param current_ma Output current in mA.
     * @return true if read successful.
     */
    bool getActiveContract(uint32_t& voltage_mv, uint32_t& current_ma);

    /**
     * @brief Read the PD_STATUS register.
     * @details Contains PD spec revision and contract status.
     * @param status_buf Buffer of at least 4 bytes to store status.
     * @return true if read successful.
     */
    bool getPdStatus(uint8_t* status_buf);

    /**
     * @brief Read the STATUS register.
     * @details Exposes connection/orientation state needed by higher-level diagnostics.
     * @param status_buf Buffer of at least 5 bytes to store status.
     * @return true if read successful.
     */
    bool getStatus(uint8_t* status_buf);

    /**
     * @brief Read the POWER_STATUS register.
     * @details Exposes the advertised Type-C current (Rp level) used to estimate
     *          available power from non-PD (Type-C only) sources.
     * @param status_buf Buffer of at least 2 bytes to store status.
     * @return true if read successful.
     */
    bool getPowerStatus(uint8_t* status_buf);

    /**
     * @brief Send a GPPI message and read the resulting message buffer.
     * @details Executes the TI 'GPPI' and 'MBRd' 4CC tasks synchronously.
     * @param gppi_header Encoded GPPI header written to DATA1 byte 0..1.
     * @param payload Optional payload bytes written after the GPPI header.
     * @param payload_len Number of payload bytes.
     * @param out_buf Buffer receiving the raw message-buffer payload bytes.
     * @param max_read_len Maximum number of payload bytes to fetch from MBRd.
     * @param actual_read_len Optional output for the message size returned by MBRd.
     * @return true when GPPI succeeds and MBRd returns a non-zero payload length.
     */
    bool sendGppiAndRead(uint16_t gppi_header,
                         const uint8_t* payload,
                         uint8_t payload_len,
                         uint8_t* out_buf,
                         uint8_t max_read_len,
                         uint16_t* actual_read_len = nullptr);

    /**
     * @brief Request USB PD Manufacturer Info from a PD partner.
     * @param frame_type SOP for the source partner; other frame types are controller-dependent.
     * @param out_buf Buffer receiving the raw Manufacturer Info response payload.
     * @param max_read_len Maximum number of payload bytes to fetch from MBRd.
     * @param actual_read_len Optional output for the message size returned by MBRd.
     * @return true when the Manufacturer Info exchange succeeds and returns payload bytes.
     */
    bool getManufacturerInfo(tps26750_gppi_frame_enum frame_type,
                             uint8_t* out_buf,
                             uint8_t max_read_len,
                             uint16_t* actual_read_len = nullptr);

    /**
     * @brief Reads the list of available power contracts offered by the source.
     * @details Reads both SPR (PDO 1-7) and EPR (PDO 8-13) capabilities.
     * @param caps Array to store the parsed capabilities.
     * @param max_caps Size of the 'caps' array (TPS26750 supports up to 13 total).
     * @return Number of capabilities found.
     */
    uint8_t getSourceCapabilities(TPS26750_SourceCapability* caps, uint8_t max_caps);

    // ------------------------------------------------------------------------
    // Contract Negotiation (Auto Negotiate Sink - Register 0x37)
    // ------------------------------------------------------------------------

    /**
     * @brief Requests a standard Fixed Voltage contract (e.g., 5V, 9V, 15V, 20V, 28V, 48V).
     * @details Sets Min/Max voltage to target +/- small tolerance, disables PPS/AVS,
     * and issues GSrC to trigger negotiation.
     *
     * @param voltage_mv Target voltage in mV.
     * @param max_current_ma Max current required in mA.
     * @return true if register write and command sent successfully.
     */
    bool requestFixedProfile(uint32_t voltage_mv, uint32_t max_current_ma);

    /**
     * @brief Requests a PPS (Programmable Power Supply) contract.
     * @details Enables PPS mode and constrains the standard fallback window to the
     * selected APDO's range, preventing the chip from picking an overlapping APDO
     * or falling back to a higher fixed rail.
     * @param voltage_mv Target voltage in mV (20mV steps).
     * @param current_ma Limit current in mA (50mA steps).
     * @param pdo_min_mv Minimum voltage of the selected PPS APDO in mV.
     * @param pdo_max_mv Maximum voltage of the selected PPS APDO in mV.
     * @return true if request sent.
     */
    bool requestPPSProfile(uint32_t voltage_mv, uint32_t current_ma,
                           uint32_t pdo_min_mv, uint32_t pdo_max_mv);

    /**
     * @brief Requests an AVS (Adjustable Voltage Supply) contract (SPR or EPR).
     * @details Enables AVS mode and constrains the standard fallback window to the
     * selected APDO's range.
     * @param voltage_mv Target voltage in mV (25mV steps).
     * @param current_ma Limit current in mA (50mA steps).
     * @param pdo_min_mv Minimum voltage of the selected AVS APDO in mV.
     * @param pdo_max_mv Maximum voltage of the selected AVS APDO in mV.
     * @return true if request sent.
     */
    bool requestAVSProfile(uint32_t voltage_mv, uint32_t current_ma,
                           uint32_t pdo_min_mv, uint32_t pdo_max_mv);

    //
    //  ERROR HANDLING
    //
    int getLastError();

private:
    bus_handle_t _bus;   // Platform-specific I2C bus handle
    uint8_t      _addr;  // 7-bit I2C address
    int          _error;

    static const uint32_t TASK_WAIT_TIMEOUT_MS = 1500;

    // Helper to block until CMD1 is cleared by the internal task engine.
    bool waitForCommandClear(uint32_t timeout_ms = TASK_WAIT_TIMEOUT_MS);

    // Helper to read/modify/write the large AUTONEGOTIATE_SINK register (24 bytes)
    bool modifySinkRegister(uint32_t min_v, uint32_t max_v, uint32_t op_i,
                            uint32_t pps_v, uint32_t pps_i, bool pps_en,
                            uint32_t avs_v, uint32_t avs_i, bool avs_en);
};
