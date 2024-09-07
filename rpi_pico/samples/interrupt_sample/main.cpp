// #define I2S_DEBUG

// Include local definition
#include <math.h>
#ifdef I2S_DEBUG
#include <stdio.h>
#endif

#include "adau1361.hpp"
#include "adau1361lower.hpp"
#include "duplexslavei2s.hpp"
#include "hardware/pio.h"
#include "i2cmaster.hpp"
#include "pico/binary_info.h"
#include "sdkwrapper.hpp"

int main() {
  const unsigned int adau1361_i2c_address = 0x38;
  const unsigned int i2c_clock = 100 * 1000;  // Hz.
  const unsigned int i2c_scl_pin = 7;
  const unsigned int i2c_sda_pin = 6;
  const unsigned int mclock = 12'000'000;  // Hz
  const unsigned int fs = 48'000;          // Hz

  /*
   * Pin usage of I2S. These pins must be consecutive.
   * The usage is defined by its relatvie position.
   *
   * 10 : SDOUT
   * 11 : SDIN
   * 12 : BCLK (IN)
   * 13 : WS (IN)
   */
  const unsigned int I2S_GPIO_PIN_BASE = 10;
  const unsigned int I2S_GPIO_PIN_DEBUG = 15;

  ::pico_driver::SDKWrapper sdk;

  // Init USB-Serial port by 9600bps, 1stop bit, 8bit.
  // add following lines to CMakeLists.txt
  // to enable usb-seial and disable serial.
  //     pico_enable_stdio_usb($ { PROJECT_NAME } 1)
  //     pico_enable_stdio_uart($ { PROJECT_NAME } 0)
  sdk.stdio_init_all();

  bi_decl(bi_program_description(
      "Working with UMB-ADAU1361A board. ADAU1361A I2C address is 0x38."));
  bi_decl(bi_program_url("https://github.com/suikan4github/duplex-i2s-pico"));
  bi_decl(bi_2pins_with_func(i2c_scl_pin, i2c_sda_pin, GPIO_FUNC_I2C));
  bi_decl(bi_4pins_with_names(
      I2S_GPIO_PIN_BASE, "I2S SDO", I2S_GPIO_PIN_BASE + 1, "I2S_SDI",
      I2S_GPIO_PIN_BASE + 2, "I2S BCLK", I2S_GPIO_PIN_BASE + 3, "I2S WS"));
  bi_decl(bi_1pin_with_name(I2S_GPIO_PIN_DEBUG, "DEBUG"));

#ifdef I2S_DEBUG
  // Delay cound down to connect the serial termail for debugging.
  uint const count = 7;
  printf("%d\n", count);
  for (size_t i = 0; i < count; i++) {
    sleep_ms(1000);
    printf("%d\n", count - i - 1);
  }
  printf("Go!\n");
#endif

  // Prepare the Audio CODEC.
  ::pico_driver::I2CMaster i2c(sdk, *i2c1, i2c_clock, i2c_scl_pin, i2c_sda_pin);
  ::codec::Adau1361Lower codec_lower(i2c, adau1361_i2c_address);
  ::codec::Adau1361 codec(fs, mclock, codec_lower);

  // Use RasPi Pico on-board LED.
  // 1=> Turn on, 0 => Turn pff.
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  sdk.gpio_init(LED_PIN);
  sdk.gpio_set_dir(LED_PIN, true);
  // Debug pin to watch the processing time by oscillo scope.
  // This pin is "H" during the audio processing.
  sdk.gpio_init(I2S_GPIO_PIN_DEBUG);
  sdk.gpio_set_dir(I2S_GPIO_PIN_DEBUG, true);

  // CODEC initializaiton and run.
  codec.Start();
  // Un mute the codec to enable the sound. Default gain is 0dB.
  codec.Mute(codec::Adau1361::LineInput, false);        // unmute
  codec.Mute(codec::Adau1361::HeadphoneOutput, false);  // unmute

  // I2S Initialization. We run the I2S PIO program from here.
  // We have to wait for the RX FIFO ASAP.
  PIO i2s_pio = pio0;
  uint i2s_sm = 0;
#if 0
  uint i2s_offset = sdk.pio_add_program(i2s_pio, &duplex_slave_i2s_program);
  ::duplex_slave_i2s_program_init(i2s_pio, i2s_sm, i2s_offset,
                                  I2S_GPIO_PIN_BASE);
#else
  ::pico_driver::DuplexSlaveI2S i2s(sdk, i2s_pio, I2S_GPIO_PIN_BASE);
  i2s.Start();
#endif

  // Audio talk thorough
  while (true) {
    // Get Left/Right I2S samples from RX FIFO.
    // Wait until the FIFO is ready.
    int32_t left_sample = (int32_t)sdk.pio_sm_get_blocking(i2s_pio, i2s_sm);
    int32_t right_sample = (int32_t)sdk.pio_sm_get_blocking(i2s_pio, i2s_sm);
    // Signaling the start of processing to the external pin.
    // We have to complete the processing within 1 sample time.
    sdk.gpio_put(I2S_GPIO_PIN_DEBUG, true);

// __arm__ predefined macro is defined by compiler if the target is
// ARM 32bit architecture.
// RP2350A has dual architecture :
// - ARM Cortex-M33 with FPU
// - RV32I without FPU
// So, we use the floating point math only for the ARM architecture.
#if (defined(PICO_RP2350A) && defined(__arm__))
    left_sample = static_cast<int32_t>(left_sample * 0.7f);
    right_sample = static_cast<int32_t>(right_sample * 0.7f);
#else
    left_sample /= 2;
    right_sample /= 2;
#endif

    // Put Left/Right I2S samples to TX FIFO.
    sdk.pio_sm_put(i2s_pio, i2s_sm, left_sample);
    sdk.pio_sm_put(i2s_pio, i2s_sm, right_sample);
    // Signaling the end of processing to the external pin.
    sdk.gpio_put(I2S_GPIO_PIN_DEBUG, false);
  }
}
