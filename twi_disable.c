#include <avr/io.h>
#include <util/twi.h>
#include "twi.h"
#include "twi_int.h"

TWI_STATUS twi_disable()
{
  PORTC &= ~(_BV(PORTC5) | _BV(PORTC4)); // Disable pull-up resistors
  TWCR = 0;
  return TWI_OK;
}

