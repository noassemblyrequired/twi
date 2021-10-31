#ifndef __TWI_INT_H__
#define __TWI_INT_H__

#include <stdint.h>

#include "twi.h"

typedef enum
{
  TWI_MODE_NO_MODE,
  TWI_MODE_MSTR_TX,
  TWI_MODE_MSTR_RX,
  TWI_MODE_SLAV_TX,
  TWI_MODE_SLAV_RX
} TWI_MODE;

typedef enum
{
  TWI_STATE_NOT_INIT  = 0,
  TWI_STATE_IDLE      = 1,
  TWI_STATE_BUSY      = 2,
  TWI_STATE_REP_START = 4,
  TWI_STATE_STOPPING  = 8,
} TWI_STATE;

typedef struct
{
  // Master Transmit/Receive
  uint8_t*  buffer;
  uint8_t   buffer_ix;
  uint8_t   buffer_sz;
  uint8_t   address;
  uint8_t   tw_status;
  uint8_t   timeout;
  TWI_STATE state;

  TWI_MODE  mode;
  
  // Master Mode Callbacks
  TWI_MASTER_COMPLETE complete_callback;
  TWI_MASTER_NACK nack_callback;

  // Slave Mode Callbacks
  TWI_SLAVE_SLA sla_callback;
  TWI_SLAVE_RX rx_callback;
  TWI_SLAVE_STOP stop_callback;
  TWI_SLAVE_TX tx_callback;
  TWI_SLAVE_LAST_DATA last_data_callback;
} TWI_DATA;

extern volatile TWI_DATA data;
void _twi_timeout(uint8_t reset);
TWI_STATUS _twi_wait_for_ready();
TWI_STATUS _twi_wait_for_rep_start();
TWI_STATUS _twi_master(uint8_t address, TWI_MASTER_RW* rw_data, uint8_t operation);

#endif // __TWI_INT_H__

