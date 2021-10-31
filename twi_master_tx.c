#include <stdint.h>
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#include "twi.h"
#include "twi_int.h"

TWI_STATUS twi_master_tx(uint8_t address, TWI_MASTER_RW* tx_data)
{
  return _twi_master(address, tx_data, TW_WRITE);
}
