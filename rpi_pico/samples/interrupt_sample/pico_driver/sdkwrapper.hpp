#ifndef __DRIVER_PICOWRAPPER_HPP__
#define __DRIVER_PICOWRAPPER_HPP__

#if __has_include("pico/stdlib.h")
// For Pico
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#else
// Alternative include and definition for Unix/Win32
typedef int i2c_inst_t;
typedef int gpio_function_t;
typedef unsigned uint;
typedef unsigned int PIO;
typedef unsigned int pio_sm_config;
typedef unsigned int clock_handle_t;
#include <stdint.h>
#include <stdlib.h>
#endif
namespace pico_driver {
/**
 * @brief Wrapper class for the RasPi Pico SDK functions.
 * @details
 * We need following line in the CMakeLitst.txt
 * @code
 *         target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_i2c)
 * @endcode
 */
class SDKWrapper {
 public:
  virtual ~SDKWrapper() {}
  /* ***************************************************************************
   *                              House Keeping
   * ***************************************************************************
   */

  /**
   * @brief Initialize all of the present standard stdio types
   * that are linked into the binary.
   * @returns
   * true if at least one output was successfully initialized, false otherwise.
   * @details
   * Call this method once you have set up your clocks
   * to enable the stdio support for UART, USB, semihosting,
   * and RTT based on the presence of the respective libraries in the binary.
   *
   * When stdio_usb is configured, this method can be optionally made to block,
   * waiting for a connection via the variables specified in stdio_usb_init
   * (i.e. PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
   */
  virtual bool stdio_init_all(void);

  /**
   * @brief Wait for the given number of milliseconds before returning.
   * @param ms the number of milliseconds to sleep
   */
  virtual void sleep_ms(uint32_t ms);

  /**
   * @brief Get the current frequency of the specified clock.
   *
   * @param clock Clock designation by clock_handle_t type.
   * @return uint32_t Clock frequency in Hz
   * @details
   * Pass clock_sys to the clock parameter to get the system clock.
   */
  virtual uint32_t clock_get_hz(clock_handle_t clock);

  /* ***************************************************************************
   *                              GPIO
   * ***************************************************************************
   */

  /**
   * @brief Initialise a GPIO for (enabled I/O and set func to
   * GPIO_FUNC_SIO)
   * @param gpio GPIO number.
   * @details
   * Clear the output enable (i.e. set to input). Clear any output value.
   */
  virtual void gpio_init(uint gpio);

  /**
   * @brief Select GPIO function.
   * @param gpio GPIO number
   * @param fn Which GPIO function select to use from list gpio_function.
   */
  virtual void gpio_set_function(uint gpio, gpio_function_t fn);
  /**
   * @brief Set a single GPIO direction.
   * @param gpio GPIO number.
   * @param out true for out, false for in.
   */
  virtual void gpio_set_dir(uint gpio, bool out);

  /**
   * @brief Drive a single GPIO high/low.
   * @param gpio GPIO number.
   * @param bool If false clear the GPIO, otherwise set it.
   */
  virtual void gpio_put(uint gpio, bool value);

  /**
   * @brief Get state of a single specified GPIO.
   * @param gpio GPIO number.
   * @returns Current state of the GPIO. 0 for low, non-zero for high
   */
  virtual bool gpio_get(uint gpio);

  /**
   * @brief Set specified GPIO to be pulled up.
   * @param gpio GPIO number.
   */
  virtual void gpio_pull_up(uint gpio);

  /* ***************************************************************************
   *                              I2C
   * ***************************************************************************
   */

  /**
   * @brief Initialise the I2C HW block.
   * @param i2c Either i2c0 or i2c1
   * @param baudrate Baudrate in Hz (e.g. 100kHz is 100000)
   * @returns Actual set baudrate.
   */
  virtual uint i2c_init(i2c_inst_t *i2c, uint baudrate);

  /**
   * @brief Disable the I2C HW block.
   * @param i2c Either i2c0 or i2c1
   * @details
   * Disable the I2C again if it is no longer used.
   * Must be reinitialised before being used again.
   */
  virtual void i2c_deinit(i2c_inst_t *i2c);

  /**
   * @brief Attempt to read specified number of bytes from address,
   * blocking.
   * @param i2c Either i2c0 or i2c1
   * @param addr 7-bit address of device to read from
   * @param dst Pointer to buffer to receive data
   * @param len Length of data in bytes to receive
   * @param nostop If true, master retains control of the bus at the end
   * of the transfer (no Stop is issued), and the next transfer will begin
   * with a Restart rather than a Start.
   * @returns Number of bytes read, or PICO_ERROR_GENERIC if address
   * not acknowledged or no device present.
   */

  virtual int i2c_read_blocking(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst,
                                size_t len, bool nostop);
  /**
   * @brief Attempt to write specified number of bytes to address, blocking.
   * @param i2c Either i2c0 or i2c1
   * @param addr 7-bit address of device to write to
   * @param src Pointer to data to send
   * @param len Length of data in bytes to send
   * @param nostop If true, master retains control of the bus at the end of
   * the transfer (no Stop is issued), and the next transfer will begin with
   * a Restart rather than a Start.
   * @returns Number of bytes written, or PICO_ERROR_GENERIC
   * if address not acknowledged, no device present.
   */
  virtual int i2c_write_blocking(i2c_inst_t *i2c, uint8_t addr,
                                 const uint8_t *src, size_t len, bool nostop);

  /* ***************************************************************************
   *                              PIO
   * ***************************************************************************
   */

  /**
   * @brief Use a state machine to set the same pin direction for multiple
   * consecutive pins for the PIO instance.
   *
   * @param pio The PIO instance; e.g. pio0 or pio1.
   * @param sm State machine index (0..3) to use.
   * @param pins_base the first pin to set a direction for.
   * @param pin_count the count of consecutive pins to set the direction for.
   * @param is_out the direction to set; true = out, false = inl
   * @return int PICO_OK (0) on success, error code otherwise.
   * @details
   * This method repeatedly reconfigures the target state machine’s pin
   * configuration and executes 'set' instructions to set the pin direction on
   * consecutive pins, before restoring the state machine’s pin configuration to
   * what it was.
   *
   * This method is provided as a convenience to set initial pin directions, and
   * should not be used against a state machine that is enabled.
   */
  virtual int pio_sm_set_consecutive_pindirs(PIO pio, uint sm, uint pins_base,
                                             uint pin_count, bool is_out);
  /**
   * @brief Setup the function select for a GPIO to use output from the given
   * PIO instance.
   *
   * @param pio The PIO instance; e.g. pio0 or pio1.
   * @param pin the GPIO pin whose function select to set.
   * @details
   * PIO appears as an alternate function in the GPIO muxing, just like an SPI
   * or UART. This function configures that multiplexing to connect a given PIO
   * instance to a GPIO. Note that this is not necessary for a state machine to
   * be able to read the input value from a GPIO, but only for it to set the
   * output value or output enable.
   */
  virtual void pio_gpio_init(PIO pio, uint pin);

  /**
   * @brief  Set the 'out' pins in a state machine configuration.
   * @param c Pointer to the configuration structure to modify.
   * @param out_base 0-31 First pin to set as output.
   * @param out_count 0-32 Number of pins to set.
   * @details  'out' pins can overlap with the 'in', 'set' and 'sideset' pins
   */
  virtual void sm_config_set_out_pins(pio_sm_config *c, uint out_base,
                                      uint out_count);

  /**
   * @brief Set the base fpr the 'in' pins in a state machine configuration.
   *
   * @param c Pointer to the configuration structure to modify.
   * @param in_base 0-31 First pin to use as input.
   * @details
   * 'in' pins can overlap with the 'out', 'set' and 'sideset' pins.
   */
  virtual void sm_config_set_in_pin_base(pio_sm_config *c, uint in_base);

  /**
   * @brief Set the count of 'in' pins in a state machine configuration.
   *
   * @param c Pointer to the configuration structure to modify.
   * @param in_count 0-32 Number of pins to set.
   * @details
   * 'in' pins can overlap with the 'out', 'set' and 'sideset' pins.
   */

  virtual void sm_config_set_in_pin_count(pio_sm_config *c, uint in_count);
  /**
   * @brief Set the state machine clock divider (from a floating point value)
   * in a state machine configuration.
   *
   * @param c Pointer to the configuration structure to modify
   * @param div The fractional divisor to be set. 1 for full speed. An integer
   * clock divisor of n will cause the state machine to run 1 cycle in every n.
   * Note that for small n, the jitter introduced by a fractional divider
   * (e.g. 2.5) may be unacceptable although it will depend on the use case.
   * @details
   * The clock divider slows the state machine’s execution by masking the system
   * clock on some cycles, in a repeating pattern, so that the state machine
   * does not advance. Effectively this produces a slower clock for the state
   * machine to run from, which can be used to generate e.g. a particular UART
   * baud rate. See the datasheet for further detail.
   */
  virtual void sm_config_set_clkdiv(pio_sm_config *c, float div);

  /**
   * @brief Setup 'in' shifting parameters in a state machine configuration.
   *
   * @param c Pointer to the configuration structure to modify.
   * @param shift_right true to shift ISR to right, false to shift ISR to left.
   * @param autopush whether autopush is enabled.
   * @param push_threshold threshold in bits to shift in before auto/conditional
   * re-pushing of the ISR
   */
  virtual void sm_config_set_in_shift(pio_sm_config *c, bool shift_right,
                                      bool autopush, uint push_threshold);

  /**
   * @brief Setup 'out' shifting parameters in a state machine configuration.
   *
   * @param c Pointer to the configuration structure to modify.
   * @param shift_right true to shift OSR to right, false to shift OSR to left.
   * @param autopull whether autopull is enabled.
   * @param push_threshold threshold in bits to shift out before
   * auto/conditional re-pulling of the OSR
   */
  virtual void sm_config_set_out_shift(pio_sm_config *c, bool shift_right,
                                       bool autopull, uint push_threshold);

  /**
   * @brief Resets the state machine to a consistent state, and configures it
   * @param pio The PIO instance; e.g.  pio0 or pio1
   * @param sm State machine index (0..3)
   * @param initial_pc the initial program memory offset to run from
   * @param config the configuration to apply (or NULL to apply defaults)
   * @return PICO_OK, or < 0 for an error (see pico_error_codes)
   * @details
   * This method:
   * @li Disables the state machine (if running)
   * @li Clears the FIFOs
   * @li Applies the configuration specified by 'config'
   * @li Resets any internal state e.g. shift counters
   * @li Jumps to the initial program location given by 'initial_pc'
   *
   * The state machine is left disabled on return from this call.
   *
   */
  virtual int pio_sm_init(PIO pio, uint sm, uint initial_pc,
                          const pio_sm_config *config);

  /**
   * @brief Write a word of data to a state machine's TX FIFO
   *
   * @param pio The PIO instance; e.g. pio0 or pio1
   * @param sm State machine index (0..3)
   * @param data the 32 bit data value
   * @details
   * This is a raw FIFO access that does not check for fullness. If the FIFO is
   * full, the FIFO contents and state are not affected by the write attempt.
   * Hardware sets the TXOVER sticky flag for this FIFO in FDEBUG, to indicate
   * that the system attempted to write to a full FIFO.
   */
  virtual void pio_sm_put(PIO pio, uint sm, uint32_t data);

  /**
   * @brief Enable or disable a PIO state machine.
   *
   * @param pio The PIO instance; e.g. pio0 or pio1
   * @param sm State machine index (0..3)
   * @param enabled true to enable the state machine; false to disable
   */
  virtual void pio_sm_set_enabled(PIO pio, uint sm, bool enabled);
};

#if __has_include(<gmock/gmock.h>)
class MockSDKWrapper : public SDKWrapper {
 public:
  MOCK_METHOD0(stdio_init_all, bool(void));
  MOCK_METHOD1(sleep_ms, void(uint32_t ms));
  MOCK_METHOD1(clock_get_hz, uint32_t(clock_handle_t clock));

  MOCK_METHOD2(gpio_set_function, void(uint gpio, gpio_function_t fn));
  MOCK_METHOD1(gpio_init, void(uint gpio));
  MOCK_METHOD2(gpio_set_dir, void(uint gpio, bool out));
  MOCK_METHOD2(gpio_put, void(uint gpio, bool value));
  MOCK_METHOD1(gpio_get, bool(uint gpio));
  MOCK_METHOD1(gpio_pull_up, void(uint gpio));

  MOCK_METHOD2(i2c_init, uint(i2c_inst_t *i2c, uint baudrate));
  MOCK_METHOD1(i2c_deinit, void(i2c_inst_t *i2c));
  MOCK_METHOD5(i2c_read_blocking, int(i2c_inst_t *i2c, uint8_t addr,
                                      uint8_t *dst, size_t len, bool nostop));
  MOCK_METHOD5(i2c_write_blocking,
               int(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src,
                   size_t len, bool nostop));

  MOCK_METHOD5(pio_sm_set_consecutive_pindirs,
               int(PIO pio, uint sm, uint pins_base, uint pin_count,
                   bool is_out));
  MOCK_METHOD2(pio_gpio_init, void(PIO pio, uint pin));
  MOCK_METHOD3(sm_config_set_out_pins,
               void(pio_sm_config *c, uint out_base, uint out_count));
  MOCK_METHOD2(sm_config_set_in_pin_base, void(pio_sm_config *c, uint in_base));
  MOCK_METHOD2(sm_config_set_in_pin_count,
               void(pio_sm_config *c, uint in_count));
  MOCK_METHOD2(sm_config_set_clkdiv, void(pio_sm_config *c, float div));
  MOCK_METHOD4(sm_config_set_in_shift,
               void(pio_sm_config *c, bool shift_right, bool autopush,
                    uint push_threshold));
  MOCK_METHOD4(sm_config_set_out_shift,
               void(pio_sm_config *c, bool shift_right, bool autopull,
                    uint push_threshold));
  MOCK_METHOD4(pio_sm_init, int(PIO pio, uint sm, uint initial_pc,
                                const pio_sm_config *config));
  MOCK_METHOD3(pio_sm_put, void(PIO pio, uint sm, uint32_t data));
  MOCK_METHOD3(pio_sm_set_enabled, void(PIO pio, uint sm, bool enabled));
};
#endif  // __has_include(<gmock/gmock.h>)
};  // namespace pico_driver

#endif  // __DRIVER_PICOWRAPPER_HPP__
