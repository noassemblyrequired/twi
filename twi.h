/**
 * \file twi.h
 */
#ifndef __TWI_H__
#define __TWI_H__

#include <stdint.h>
#include <util/twi.h>

#ifndef DELAY_US
#define DELAY_US 5
#endif

/**
 * @enum F_SCL_FREQ
 * @brief Frequency of SCL
 */
typedef enum
{
  F_SCL_100_kHz  = 100,   /*!< Use 100kHz clock */
  F_SCL_400_kHz  = 400    /*!< Use 400kHz clock */
} F_SCL_FREQ;

/**
 * @brief Prescaler values
 */
typedef enum
{
  TWI_PRESCALER_BY_1   = 0,   /*!< No prescaler           */
  TWI_PRESCALER_BY_4   = 1,   /*!< Divide by 4 prescaler  */
  TWI_PRESCALER_BY_16  = 2,   /*!< Divide by 16 prescaler */
  TWI_PRESCALER_BY_64  = 3,   /*!< Divide by 64 prescaler */
} TWI_PRESCALER;

/**
 * @brief Actions to perform, returned by callbacks to the TWI API.
 * These values can be bitwise OR'd together.
 */
typedef enum
{
  TWI_ACTION_ACK    = 0x01, /*!< Issue or Enable ACK        */
  TWI_ACTION_NACK   = 0x02, /*!< Issue NACK or Disable ACK  */
  TWI_ACTION_START  = 0x04, /*!< Issue a START condition    */
  TWI_ACTION_STOP   = 0x08, /*!< Issue a STOP condition     */
  TWI_ACTION_CONT   = 0x10  /*!< Continue                   */
} TWI_ACTION;

typedef enum
{
  TWI_OK                    = 1,
  TWI_START                 = TW_START,
  TWI_REP_START             = TW_REP_START,
  TWI_MT_SLA_ACK            = TW_MT_SLA_ACK,
  TWI_MT_SLA_NACK           = TW_MT_SLA_NACK,
  TWI_MT_DATA_ACK           = TW_MT_DATA_ACK,
  TWI_MT_DATA_NACK          = TW_MT_DATA_NACK,
  TWI_MT_ARB_LOST           = TW_MT_ARB_LOST,
  TWI_MR_ARB_LOST           = TW_MR_ARB_LOST,
  TWI_MR_SLA_ACK            = TW_MR_SLA_ACK,
  TWI_MR_SLA_NACK           = TW_MR_SLA_NACK,
  TWI_MR_DATA_ACK           = TW_MR_DATA_ACK,
  TWI_MR_DATA_NACK          = TW_MR_DATA_NACK,
  TWI_ST_SLA_ACK            = TW_ST_SLA_ACK,
  TWI_ST_ARB_LOST_SLA_ACK   = TW_ST_ARB_LOST_SLA_ACK,
  TWI_ST_DATA_ACK           = TW_ST_DATA_ACK,
  TWI_ST_DATA_NACK          = TW_ST_DATA_NACK,
  TWI_ST_LAST_DATA          = TW_ST_LAST_DATA,
  TWI_SR_SLA_ACK            = TW_SR_SLA_ACK,
  TWI_SR_ARB_LOST_SLA_ACK   = TW_SR_ARB_LOST_SLA_ACK,
  TWI_SR_GCALL_ACK          = TW_SR_GCALL_ACK,
  TWI_SR_ARB_LOST_GCALL_ACK = TW_SR_ARB_LOST_GCALL_ACK,
  TWI_SR_DATA_ACK           = TW_SR_DATA_ACK,
  TWI_SR_DATA_NACK          = TW_SR_DATA_NACK,
  TWI_SR_GCALL_DATA_ACK     = TW_SR_GCALL_DATA_ACK,
  TWI_SR_GCALL_DATA_NACK    = TW_SR_GCALL_DATA_NACK,
  TWI_SR_STOP               = TW_SR_STOP,
  TWI_NO_INFO               = TW_NO_INFO,
  TWI_BUS_ERROR             = TW_BUS_ERROR,
  TWI_NOT_INIT              = 0xFD,
  TWI_NO_WAIT               = 0xFE,
  TWI_TIMEDOUT              = 0xFF
} TWI_STATUS;

/**
 * @brief Called on TWI_M[T|R]_SLA_NACK, TWI_MT_DATA_NACK, TWI_M[T|R]_ARB_LOST.
 * @param status The current status code.
 *
 * @return
 * Value of status |  TWI_ACTION_STOP | TWI_ACTION_START | TWI_ACTION_CONT
 * ---|---|---|---
 * TWI_MT_SLA_NACK | Will issue STOP condition [default] | Will issue Rep Start Condition | Will Send Data
 * TWI_MT_DATA_NACK | Will issue STOP condition [default] | Will issue Rep Start Condition | Will Send Data
 * TWI_MT_ARB_LOST | Will Release Bus [default] | Will issue START Condition | N/A
 * TWI_MR_SLA_NACK | Will issue STOP condition [default] | Will issue Rep Start Condition | N/A
 * TWI_MR_ARB_LOST | Will Release Bus [default] | Will issue START Condition | N/A
 *
 * @note Respected values returned from this function depend on value of
 *       status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_MASTER_NACK)(TWI_STATUS status);

/**
 * @brief Called when RX or TX has been completed.
 * @param status The current status code.
 *
 * @return
 *  TWI_ACTION_STOP | TWI_ACTION_START 
 * ---|---
 * Will issue STOP condition [default] | Will issue Rep Start Condition 
 */
typedef TWI_ACTION (*TWI_MASTER_COMPLETE)(TWI_STATUS status);

/**
 * @brief Called when the address on the bus matches slave address.
 * Address may be 0, if slave was configured to support general call.
 * @param address The matched address
 * @param status The current status code. Either TWI_SR_SLA_ACK,
 *               TWI_SR_ARB_LOST_SLA_ACK, TWI_ST_SLA_ACK,
 *               TWI_ST_ARB_LOST_SLA_ACK, TWI_SR_GCALL_ACK, or
 *               TWI_SR_ARB_LOST_GCALL_ACK
 *
 * @return
 * Value of status | TWI_ACTION_ACK | TWI_ACTION_NACK
 * ---|---|---
 * TWI_SR_SLA_ACK, TWI_SR_GCALL_ACK, TWI_SR_ARB_LOST_SLA_ACK, TWI_SR_ARB_LOST_GCALL_ACK | Next byte will be ACK'd | Next byte will be NACK'd
 * TW_ST_SLA_ACK, TWI_ST_ARB_LOST_SLA_ACK | N/A | N/A
 * 
 * @note Respected values returned from this function depend on the value of 
 *        status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_SLAVE_SLA)(uint8_t address, TWI_STATUS status);

/**
 * @brief Called when RX'd data byte
 * @param data The received data.
 * @param status The current status code. Either TWI_SR_DATA_ACK
 *
 * @return
 * Value of status| TWI_ACTION_ACK | TWI_ACTION_NACK |  TWI_ACTION_START 
 * ---|---|---|---
 * TWI_SR_DATA_ACK, TWI_SR_GCALL_DATA_ACK | Next byte will be ACK'd | Next byte will be NACK'd | N/A 
 * TWI_SR_DATA_NACK, TWI_SR_GCALL_DATA_NACK | Recognize next SLA or general call | Do not recognize next SLA or general call | Issue a Start Condition
 *
 * @notes Respected values returned from this function depend on the value of 
 *        status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_SLAVE_RX)(uint8_t data, TWI_STATUS status);

/**
 * @brief Called on STOP condition
 * @return 
 * TWI_ACTION_ACK | TWI_ACTION_NACK |  TWI_ACTION_START 
 * ---|---|---
 * Recognize next SLA or general call [default] | Do not recognize next SLA or general call | Issue a Start Condition
 *
 * @note Respected values returned from this function depend on the value of 
 *        status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_SLAVE_STOP)();

/**
 * @brief Called when master requests a byte of data
 * @param[out] data Put value to send to master here
 *
 * @return 
 * TWI_ACTION_ACK | TWI_ACTION_NACK |  TWI_ACTION_START 
 * ---|---|---
 * ACK should be received from master | NACK should be received from master
 *
 * @note Respected values returned from this function depend on the value of 
 *        status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_SLAVE_TX)(uint8_t* data);

/**
 * @brief Called when last data byte has been transmitted or NACK received.
 * @param status Value is either TW_ST_DATA_NACK, or TW_ST_LAST_DATA
 *
 * @return
 * TWI_ACTION_ACK | TWI_ACTION_NACK |  TWI_ACTION_START 
 * ---|---|---
 * Recognize next SLA or general call [default] | Do not recognize next SLA or general call | Issue a Start Condition
 *
 * @note Respected values returned from this function depend on the value of 
 *        status. Values not listed are N/A.
 */
typedef TWI_ACTION (*TWI_SLAVE_LAST_DATA)(TWI_STATUS status);

typedef struct
{
  F_SCL_FREQ F_scl_kHz;
  uint8_t  timeout;
  TWI_MASTER_COMPLETE complete_callback;
  TWI_MASTER_NACK nack_callback;
} TWI_INIT;

typedef struct
{
  uint8_t twi_baud;
  uint8_t timeout;
  TWI_PRESCALER prescaler;
  TWI_MASTER_COMPLETE complete_callback;
  TWI_MASTER_NACK nack_callback;
} TWI_INIT2;

typedef struct
{
  uint8_t* data;          /*!< The data buffer */
  uint8_t  data_sz;       /*!< Number of bytes data points to */
  uint8_t  no_start;      /*!< Don't issue a start condition. This is set to 1 if a slave mode issued a Start Condition. */
  uint8_t  posted_write;  /*!< Complete a posted write, do not wait for all bytes to be issued to slave. */
} TWI_MASTER_RW;

typedef struct
{
  TWI_SLAVE_SLA       sla_callback;           /*!< Not required */
  TWI_SLAVE_RX        rx_callback;            /*!< Recommended */
  TWI_SLAVE_STOP      stop_callback;          /*!< Not required */
  TWI_SLAVE_TX        tx_callback;            /*!< Required */
  TWI_SLAVE_LAST_DATA last_data_callback;     /*!< Not required */
} TWI_SLAVE_CALLBACKS;

TWI_STATUS twi_set_address(uint8_t address);
TWI_STATUS twi_master(TWI_INIT* init);
TWI_STATUS twi_master2(TWI_INIT2* init);
TWI_STATUS twi_stop();
TWI_STATUS twi_disable();
TWI_STATUS twi_master_tx(uint8_t address, TWI_MASTER_RW* tx_data);
TWI_STATUS twi_master_rx(uint8_t address, TWI_MASTER_RW* rx_data);

#define TWI_SLAVE_GENERAL_CALL(address)   (((address) << 1) | 1)
#define TWI_SLAVE_NO_GENERAL_CALL(address) ((address) << 1)

/**
 * @param address The slave address shifted with optional general call bit set.
 * @param address_mask The address mask if slave is to respond to multiple
 *    addresses. Set to 0 for single address.
 * @param callbacks The callbacks for the slave.
 */
TWI_STATUS twi_slave(uint8_t address, uint8_t address_mask, TWI_SLAVE_CALLBACKS* callbacks);

#endif // __TWI_H__
