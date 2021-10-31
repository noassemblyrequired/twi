#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include "twi.h"
#include "twi_int.h"

TWI_STATUS twi_master_rx(uint8_t address, TWI_MASTER_RW* rx_data)
{
  return _twi_master(address, rx_data, TW_READ);
}

