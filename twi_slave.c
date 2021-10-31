#include <avr/io.h>
#include <util/twi.h>
#include <stdio.h>
#include "twi.h"
#include "twi_int.h"

TWI_STATUS twi_slave(uint8_t address, uint8_t address_mask, TWI_SLAVE_CALLBACKS* callbacks)
{
  data.rx_callback = callbacks->rx_callback;
  data.stop_callback = callbacks->stop_callback;
  data.sla_callback = callbacks->sla_callback;
  data.tx_callback = callbacks->tx_callback;
  data.last_data_callback = callbacks->last_data_callback;

  TWAR = address;
  TWAMR = address_mask << 1;

  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

  return TWI_OK;
}
