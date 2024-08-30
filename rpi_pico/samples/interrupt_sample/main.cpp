// Include local definition
#include <stdio.h>

#include "adau1361.hpp"
#include "adau1361lower.hpp"
#include "duplex_i2s.pio.h"
#include "duplex_i2s_program_init.hpp"
#include "hardware/pio.h"
#include "i2cmaster.hpp"
#include "picowrapper.hpp"

int main() {
  const unsigned int adau1361_i2c_address = 0x38;
  const unsigned int i2c_clock = 100 * 1000;  // Hz.
  const unsigned int i2c_scl_pin = 7;
  const unsigned int i2c_sda_pin = 6;
  const unsigned int mclock = 12'000'000;  // Hz
  const unsigned int fs = 48'000;          // Hz

  /*
   * 10 : SDOUT
   * 11 : SDIN
   * 12 : BCLK
   * 13 : WS
   */
  const unsigned int I2S_GPIO_PIN_BASE = 10;

  ::pico_driver::SDKWrapper sdk;

  // Init USB-Serial port by 9600bps, 1stop bit, 8bit.
  // add following lines to CMakeLists.txt
  // to enable usb-seial and disable serial.
  //     pico_enable_stdio_usb($ { PROJECT_NAME } 1)
  //     pico_enable_stdio_uart($ { PROJECT_NAME } 0)
  sdk.stdio_init_all();

  uint const count = 7;
  printf("%d\n", count);
  for (size_t i = 0; i < count; i++) {
    sleep_ms(1000);
    printf("%d\n", count - i - 1);
  }
  printf("Go!\n");

  ::pico_driver::I2CMaster i2c(sdk, *i2c1, i2c_clock, i2c_scl_pin, i2c_sda_pin);
  ::codec::Adau1361Lower codec_lower(i2c, adau1361_i2c_address);
  ::codec::Adau1361 codec(fs, mclock, codec_lower);

  printf("CODEC Initialization.\n");
  // CODEC initializaiton.
  codec.Start();
  codec.Mute(codec::Adau1361::LineInput, false);        // unmute
  codec.Mute(codec::Adau1361::HeadphoneOutput, false);  // unmute

  printf("I2S Initialization.\n");
  // I2S Initialization.
  PIO i2s_pio = pio0;
  uint i2s_sm = 0;
  uint i2s_offset = pio_add_program(i2s_pio, &duplex_i2s_program);
  duplex_i2s_program_init(i2s_pio, i2s_sm, i2s_offset, I2S_GPIO_PIN_BASE);

  printf("LED Initialization.\n");
  // Use RasPi Pico on-board LED.
  // 1=> Turn on, 0 => Turn pff.
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  sdk.gpio_init(LED_PIN);
  sdk.gpio_set_dir(LED_PIN, true);

  printf("System clock is  %d Hz.\n", clock_get_hz(clk_sys));
  float div = (clock_get_hz(clk_sys) / (48'000'000));
  printf("Divsion factor is  %f .\n", div);

  printf("Audio Transfering.\n");
  bool led = false;
  // Audio talk thorough
  while (true) {
    // Get Left/Right I2S samples from RX FIFO.
    uint32_t left_sample = pio_sm_get_blocking(i2s_pio, i2s_sm);
    uint32_t right_sample = pio_sm_get_blocking(i2s_pio, i2s_sm);
    // Put Left/Right I2S samples to TX FIFO.
    pio_sm_put(i2s_pio, i2s_sm, left_sample);
    pio_sm_put(i2s_pio, i2s_sm, right_sample);
    // LED control
    sdk.gpio_put(LED_PIN, led);
    led != led;
  }
}
