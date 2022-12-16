#ifndef Adafruit_I2CDevice_h
#define Adafruit_I2CDevice_h


// SoftWire config
//#define USE_SOFT_I2C_MASTER_H_AS_PLAIN_INCLUDE
#define USE_SOFTWIRE_H_AS_PLAIN_INCLUDE
#define SCL_PIN 4// 8
#define SCL_PORT PORTB
#define SDA_PIN 5// 9
#define SDA_PORT PORTB

#define I2C_FASTMODE 1
#define I2C_TIMEOUT 1000
#define I2C_PULLUP 1
//--------------

#include <Arduino.h>
#include "../SoftWire/SoftWire.h"

///< The class which defines how we will talk to this device over I2C
class Adafruit_I2CDevice {
public:
  Adafruit_I2CDevice(uint8_t addr, SoftWire *theWire = &Wire);
  uint8_t address(void);
  bool begin(bool addr_detect = true);
  void end(void);
  bool detected(void);

  bool read(uint8_t *buffer, size_t len, bool stop = true);
  bool write(const uint8_t *buffer, size_t len, bool stop = true,
             const uint8_t *prefix_buffer = NULL, size_t prefix_len = 0);
  bool write_then_read(const uint8_t *write_buffer, size_t write_len,
                       uint8_t *read_buffer, size_t read_len,
                       bool stop = false);
  bool setSpeed(uint32_t desiredclk);

  /*!   @brief  How many bytes we can read in a transaction
   *    @return The size of the Wire receive/transmit buffer */
  size_t maxBufferSize() { return _maxBufferSize; }

private:
  uint8_t _addr;
  SoftWire *_wire;
  bool _begun;
  size_t _maxBufferSize;
  bool _read(uint8_t *buffer, size_t len, bool stop);
};

#endif // Adafruit_I2CDevice_h
