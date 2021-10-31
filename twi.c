#include <stdlib.h>
#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "twi.h"
#include "twi_int.h"

static void _twi_handle_complete();

volatile TWI_DATA data = 
{
  .buffer     = NULL,
  .buffer_ix  = 0,
  .buffer_sz  = 0,
  .tw_status  = TW_NO_INFO,
  .address    = 0,
  .timeout    = 0,
  .state      = TWI_STATE_NOT_INIT,
  .mode       = TWI_MODE_NO_MODE,

  // Master Callback Defaults
  .complete_callback  = NULL,
  .nack_callback      = NULL,

  // Slave Callback Defaults
  .sla_callback       = NULL,
  .rx_callback        = NULL,
  .stop_callback      = NULL,
  .tx_callback        = NULL,
  .last_data_callback = NULL
};

TWI_STATUS twi_master(TWI_INIT* init)
{
  TWI_INIT2 init2 = 
  {
    .twi_baud = (((F_CPU / 10e3) / init->F_scl_kHz) - 16) / 2,
    .prescaler = TWI_PRESCALER_BY_1,
    .timeout = init->timeout,
    .complete_callback = init->complete_callback,
    .nack_callback = init->nack_callback,
  };

  return twi_master2(&init2);
}

TWI_STATUS twi_master2(TWI_INIT2* init)
{
  TWSR = (init->prescaler & ~TW_STATUS_MASK); 
  TWBR = init->twi_baud;
  data.state = TWI_STATE_IDLE;
  data.timeout = init->timeout;
  data.complete_callback = init->complete_callback;
  data.nack_callback = init->nack_callback;
  PORTC |= _BV(PORTC5) | _BV(PORTC4); // Enable pull-up resistors, JIC
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);
  return TWI_OK;
}

TWI_STATUS twi_reset()
{
  uint8_t twbr = TWBR;
  uint8_t prescaler = TWSR & ~TW_STATUS_MASK;
  TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA));
  TWBR = twbr;
  TWSR = prescaler;
  data.state = TWI_STATE_IDLE;
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);
  return TWI_OK;
}

TWI_STATUS twi_stop()
{
  data.state = TWI_STATE_STOPPING;
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN) | _BV(TWEA) | _BV(TWIE);
  uint8_t i = 0;
  for(; i < data.timeout; ++i)
  {
    if ((TWCR & _BV(TWSTO)) == 0)
      break;
    _delay_us(1);
  }

  if (i == data.timeout)
  {
    _twi_timeout(1);
    return TWI_TIMEDOUT;
  }
  else
  {
    data.state = TWI_STATE_IDLE;
    return TWI_OK;
  }
}

ISR(TWI_vect)
{
  data.tw_status = TW_STATUS;
  switch(data.tw_status)
  {
    case TW_START:
      TWDR = data.address;
      TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
      break;
    case TW_REP_START:
      break;

    // Master Transmit Cases
    case TW_MT_SLA_ACK:
    case TW_MT_DATA_ACK:
      if (data.buffer_ix >= data.buffer_sz)
      {
        // No more data to send, check what do to next, either 
        // - Repeated Start Condition
        // - Stop Condition
        // - Stop Condition Followed by Start Condition
        _twi_handle_complete();
      }
      else
      {
        // Load next data byte
        TWDR = data.buffer[data.buffer_ix++];
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA) | _BV(TWIE);
      }
      break;
    case TW_MT_SLA_NACK:
    case TW_MT_DATA_NACK:
      {
        // Slave NACK'd last data, check what to do next, either
        // - Send data anyway
        // - Repeated Start Condition
        // - Stop Condition
        // - Stop Condition Followed by Start Condition
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
        TWI_ACTION action = data.nack_callback ? data.nack_callback(data.tw_status) : TWI_ACTION_STOP;
        if (action & TWI_ACTION_CONT)
        {
          if (data.buffer_ix >= data.buffer_sz)
          {
            // No more data to send, check what do to next, either 
            // - Repeated Start Condition
            // - Stop Condition
            // - Stop Condition Followed by Start Condition
            _twi_handle_complete();
          }
          else
          {
            // Load next data byte
            TWDR = data.buffer[data.buffer_ix++];
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA) | _BV(TWIE);
          }
        }
        else
        {
          if (action & TWI_ACTION_STOP)
          {
            data.state = TWI_STATE_IDLE;
            twcr |= _BV(TWSTO);
          }
          if (action & TWI_ACTION_START)
          {
            data.state = TWI_STATE_REP_START;
            twcr |= _BV(TWSTA);
          }
        }

        TWCR = twcr;
      }
      break;
    case TW_MT_ARB_LOST: // this is also TW_MR_ARB_LOST
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
        TWI_ACTION action = data.nack_callback ? data.nack_callback(data.tw_status) : TWI_ACTION_STOP;
        data.state = TWI_STATE_IDLE;
        if (action & TWI_ACTION_START)
        {
          data.state = TWI_STATE_REP_START;
          twcr |= _BV(TWSTA);
        }

        TWCR = twcr;
      }
      break;

    // Master Receiver Cases
    case TW_MR_DATA_ACK:
      data.buffer[data.buffer_ix++] = TWDR;
    case TW_MR_SLA_ACK:
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        if (data.buffer_ix + 1 != data.buffer_sz) // there is room for one more
          twcr |= _BV(TWEA);
        TWCR = twcr;
      }
      break;
    case TW_MR_SLA_NACK: 
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
        TWI_ACTION action = data.nack_callback ? data.nack_callback(data.tw_status) : TWI_ACTION_STOP;
        if (action & TWI_ACTION_STOP)
        {
          data.state = TWI_STATE_IDLE;
          twcr |= _BV(TWSTO);
        }
        if (action & TWI_ACTION_START)
        {
          data.state = TWI_STATE_REP_START;
          twcr |= _BV(TWSTA);
        }

        TWCR = twcr;
      }
      break;
    case TW_MR_DATA_NACK: // last byte rx'd, nack sent
      data.buffer[data.buffer_ix] = TWDR;
      _twi_handle_complete();
      break;

    // Slave Receiver Cases
    case TW_SR_SLA_ACK:
    case TW_SR_ARB_LOST_SLA_ACK:
    case TW_SR_GCALL_ACK:
    case TW_SR_ARB_LOST_GCALL_ACK:
      {
        data.mode = TWI_MODE_SLAV_RX;
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        // if the slave has provided a callback for SLA then call it, otherwise always ack
        TWI_ACTION action = data.sla_callback ? data.sla_callback(TWDR >> 1, data.tw_status) : TWI_ACTION_ACK;
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        TWCR = twcr;
      }
      break;
    case TW_SR_DATA_ACK:
    case TW_SR_GCALL_DATA_ACK:
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        TWI_ACTION action = data.rx_callback ? data.rx_callback(TWDR, data.tw_status) : TWI_ACTION_NACK;
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        TWCR = twcr;
      }
      break;
    case TW_SR_STOP:
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        TWI_ACTION action = data.stop_callback ? data.stop_callback() : TWI_ACTION_ACK;
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        if (action & TWI_ACTION_START)
          twcr |= _BV(TWSTA);
        TWCR = twcr;
      }
      break;
    case TW_SR_DATA_NACK:
    case TW_SR_GCALL_DATA_NACK:
      {
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        TWI_ACTION action = data.rx_callback ? data.rx_callback(TWDR, data.tw_status) : TWI_ACTION_NACK;
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        if (action & TWI_ACTION_START)
          twcr |= _BV(TWSTA);
        TWCR = twcr;
      }
      break;

    // Save Transmitter Cases
    case TW_ST_SLA_ACK:
    case TW_ST_ARB_LOST_SLA_ACK:
        if (data.sla_callback)
          data.sla_callback(TWDR >> 1, data.tw_status);
        data.mode = TWI_MODE_SLAV_TX;
    case TW_ST_DATA_ACK:
      {
        uint8_t twdr;
        TWI_ACTION action = data.tx_callback(&twdr);
        TWDR = twdr;
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        TWCR = twcr;
      }
      break;
    case TW_ST_DATA_NACK:
    case TW_ST_LAST_DATA:
      { 
        uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        uint8_t action = TWI_ACTION_ACK;
        if (data.last_data_callback)
          action = data.last_data_callback(data.tw_status);
        if (action & TWI_ACTION_ACK)
          twcr |= _BV(TWEA);
        if (action & TWI_ACTION_START)
          twcr |= _BV(TWSTA);
        TWCR = twcr;
      }
      break;
    
    case TW_BUS_ERROR:
      twi_stop();
      break;
  }
}

void _twi_timeout(uint8_t reset)
{
  if (reset)
    twi_reset();
}

TWI_STATUS _twi_wait_for_ready()
{
  // Wait until state machine is idle (not reading/writing) or a repeated start
  // condition has been issued.
  uint8_t timeout_cntr = data.timeout;
  for (; timeout_cntr > 0; --timeout_cntr)
  {
    if (data.state == TWI_STATE_IDLE || data.state == TWI_STATE_REP_START)
      break;
    _delay_us(DELAY_US);
  }

  if (!timeout_cntr)
  {
    // not issuing a reset here, the state machine could still be transmitting data.
    _twi_timeout(0);
    return TWI_TIMEDOUT;
  }

  return TWI_OK;
}

TWI_STATUS _twi_wait_for_rep_start()
{
  uint8_t timeout_cntr = data.timeout;
  for (; timeout_cntr > 0; --timeout_cntr)
  {
    // wait for repeated state condition
    if (data.tw_status == TW_REP_START)
      break;
    _delay_us(DELAY_US);
  }

  if (!timeout_cntr)
  {
    _twi_timeout(1);
    return TWI_TIMEDOUT;
  }

  data.state = TWI_STATE_BUSY;
  TWDR = data.address;
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);

  return TWI_OK;
}

TWI_STATUS _twi_master(uint8_t address, TWI_MASTER_RW* rw_data, uint8_t operation)
{
  if (data.state == TWI_STATE_NOT_INIT)
    return TWI_NOT_INIT;

  data.mode = operation == TW_WRITE ? TWI_MODE_MSTR_TX : TWI_MODE_MSTR_RX;
  data.address = (address << 1) | operation;
  data.buffer = rw_data->data;
  data.buffer_sz = rw_data->data_sz;
  data.buffer_ix = 0;

  if (_twi_wait_for_ready() == TWI_TIMEDOUT)
  {
    return TWI_TIMEDOUT;
  }

  if (data.state == TWI_STATE_REP_START) // Repeated start condition has been issued
  {
    if (_twi_wait_for_rep_start() == TWI_TIMEDOUT)
    {
      return TWI_TIMEDOUT;
    }
  }
  else // IDLE, issue start condition
  {
    data.state = TWI_STATE_BUSY;
    uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
    if (!rw_data->no_start) // only include TWSTA if not requested to not send a start
      twcr |= _BV(TWSTA);

    TWCR = twcr;
  }

  if (operation == TW_WRITE && rw_data->posted_write)
  {
    return TWI_NO_WAIT;
  }

  if (_twi_wait_for_ready() == TWI_TIMEDOUT)
  {
    return TWI_TIMEDOUT;
  }

  return data.tw_status;
}

static void _twi_handle_complete()
{
  uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
  TWI_ACTION action = data.complete_callback ? data.complete_callback(data.tw_status) : TWI_ACTION_STOP;
  if (action & TWI_ACTION_STOP)
  {
    data.state = TWI_STATE_IDLE;
    twcr |= _BV(TWSTO);
  }
  if (action & TWI_ACTION_START)
  {
    data.state = TWI_STATE_REP_START;
    twcr |= _BV(TWSTA);
  }

  TWCR = twcr;
}
