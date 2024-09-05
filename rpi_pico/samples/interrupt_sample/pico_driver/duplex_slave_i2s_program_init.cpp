

#include "duplex_slave_i2s_program_init.hpp"

#include "sdkwrapper.hpp"

/**
 * @brief Initialize the duplex I2S controller.
 * @param pio One of pio0, pio1,...
 * @param sm One of 0,1,2,3.
 * @param offset of the PIO program.
 * @param pin_sdout GPIO pin number for I2S BCLK signal.
 * @details
 * The caller must set pio, sm and offset collectly, based on the PIO
 * programming manner.
 *
 * We need four GPIO pin in this function, and assumes the GPIO pins are
 * following order as ascending manner.
 * @li SDOUT as output (DAC)
 * @li SDIN as input (ADC)
 * @li BCLK as input
 * @li WS as JMP pin
 *
 * For example, assume the pin_sdout parameter is 5 (GPIO PIN 5). In this case
 * the SDOUT, SDIN, BCLK, and WS must be GPIO PIN 5, 6, 7 and 8, respectively.
 */
void duplex_slave_i2s_program_init(PIO pio, uint sm, uint offset,
                                   uint pin_sdout) {
  assert(sm < 4);
  ::pico_driver::SDKWrapper sdk;

  // PIO configuration. Set SDOUT as output. Set SDIN, BCLK and WS as input.
  sdk.pio_sm_set_consecutive_pindirs(pio, sm,
                                     pin_sdout,  // BASE GPIO pin number.
                                     1,          // 1 pin for output.
                                     true);      // true for output.
  sdk.pio_sm_set_consecutive_pindirs(pio, sm,
                                     pin_sdout + 1,  // BASE GPIO pin number.
                                     3,              // 3 pins for input.
                                     false);         // false for input.

  // Use thse four pins by PIO.
  sdk.pio_gpio_init(pio, pin_sdout);
  sdk.pio_gpio_init(pio, pin_sdout + 1);
  sdk.pio_gpio_init(pio, pin_sdout + 2);
  sdk.pio_gpio_init(pio, pin_sdout + 3);

  // State machine configuration.
  // The duplex_i2s_program_get_default_config() is generated by the
  // PIO compiler. So, there is not wrapper.
  pio_sm_config config = duplex_slave_i2s_program_get_default_config(offset);

  // Set direction of pins.
  sdk.sm_config_set_out_pins(&config,
                             pin_sdout,  // PIN_SDOUT
                             1);         // 1 pin for output.
  sdk.sm_config_set_in_pin_base(&config,
                                pin_sdout + 1);  // PIN_SDIN.
#if 0
  // The sm_config_set_in_pin_count() has bug in SDK version 2.0.0.
  // See https://github.com/raspberrypi/pico-sdk/issues/1878 for details. 
  // Actually, without this API call, program works in both RP2040 and RP2350. 
  // So, remove this code from the release version. 
  sdk.sm_config_set_in_pin_count(&config,
                                 2);                  // 2 pins for input.
#endif
  sdk.sm_config_set_jmp_pin(&config, pin_sdout + 3);  // WS

  // Set the PIO clock divider.
  // We need 96kHz stereo 32bit transfer. So the BCLCK is 96'000*2*32Hz.
  // The  clock for the duplex I2S PIO program must be 10 times or grather
  // ( See the comment in the duplex_i2s.pio ).
  // To avoid the jitter, we calculate the division factor in
  // integer.
  float div = (sdk.clock_get_hz(clk_sys) / (96'000 * 2 * 32 * 10));
  sdk.sm_config_set_clkdiv(&config, div);

  // Shift configuration : No Auto push/pull.
  // Input shift register.
  sdk.sm_config_set_in_shift(&config,
                             false,  // false to left shift.
                             false,  // true to auto push.
                             32);    // 32bit word.
  // Output shift register.
  sdk.sm_config_set_out_shift(&config,
                              false,  // false to left shift.
                              false,  // true to auto pull.
                              32);    // 32bit word.
  // Configure SM.
  sdk.pio_sm_init(pio, sm, offset, &config);

  // Placing dummy data in FIFO to avoid the underflow at first TX.
  // This must be done after calling pio_sm_init().
  // We need 2 stereo samples.
  // The first stereo sample is for the initial transfer. For this transfer,
  // main program is waiting the received signal. So, we need to fill dummy.
  // After the first data transfer, the main routine starts to process data.
  // We need time for max 1 sample. Then, we need to fill another dummy.
  sdk.pio_sm_put(pio, sm, 0);  // Put left word for the first TX.
  sdk.pio_sm_put(pio, sm, 0);  // Put right word for the first TX.
  sdk.pio_sm_put(pio, sm, 0);  // Put left word for the second TX.
  sdk.pio_sm_put(pio, sm, 0);  // Put right word for the second TX.

  // Start the state machine.
  sdk.pio_sm_set_enabled(pio, sm, true);
}
