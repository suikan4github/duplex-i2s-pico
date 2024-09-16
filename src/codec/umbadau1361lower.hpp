/**
 * @file umbadau1361lower.hpp
 *
 * @brief Lower class for the UMB-ADAU1361A.
 * @date 2024/Sep/16
 * @author Seiichi Horie
 * @copyright Copyright 2024 Seiichi Horie
 */

#ifndef _UMBADAU1361LOWER_HPP_
#define _UMBADAU1361LOWER_HPP_

#include "adau1361lower.hpp"
#include "i2cmaster.hpp"

namespace pico_driver {

/**
 * @brief lower part of the Adau1361 CODEC controller class.
 * @details
 * This class is helper class for the Adau1361 class.
 *
 * Note, all volumes are muted.
 */
class UmbAdau1361Lower : public Adau1361Lower {
 public:
  /**
   * @brief Construct a new object
   *
   * @param controller I2C master controller.
   * @param i2c_device_addr ADAU1361A 7bits I2C device address. Refer device
   * deta sheet for details.
   */
  UmbAdau1361Lower(::pico_driver::I2cMasterInterface& controller,
                   unsigned int i2c_device_addr)
      : Adau1361Lower(controller, i2c_device_addr) {}
  UmbAdau1361Lower() = delete;
  virtual ~UmbAdau1361Lower() {}

  /**
   * @brief Initialize registers for the signal routing.
   * @details
   * This is baord dependent initialization for UMB-ADAU1361A.
   *
   * Need to call after InitializeRegisters().
   */
  void ConfigureSignalPath() override;
};

}  // namespace pico_driver

#endif /* _UMBADAU1361LOWER_HPP_ */