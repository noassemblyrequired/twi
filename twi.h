/**
 * \file twi.h
 */
#ifndef __TWI_H__
#define __TWI_H__

#include <stdint.h>
#include <util/twi.h>

#ifndef DELAY_US
/**
 * @brief The number of microseconds to sleep while waiting for specific
 *        condition to be met. Pair this with TWI_INIT::timeout.
 */
#define DELAY_US 5 
#endif

// #define TWI_NO_SLAVE
// #define TWI_NO_MASTER

/**
 * @brief Construct the slave address with general call support.
 * @param address The slave address to construct
 */
#define TWI_SLAVE_GENERAL_CALL(address)   (((address) << 1) | 1)

/**
 * @brief Construct the slave address without general call support.
 * @param address The slave address to construct
 */
#define TWI_SLAVE_NO_GENERAL_CALL(address) ((address) << 1)

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

/**
 * @brief Status Codes
 */
typedef enum
{
  TWI_OK                    = 1,                          /*!< Command completed */
  TWI_START                 = TW_START,                   /*!< Start condition transmitted */
  TWI_REP_START             = TW_REP_START,               /*!< Repeated start condition transmitted */
  TWI_MT_SLA_ACK            = TW_MT_SLA_ACK,              /*!< SLA+W has been transmitted; ACK received */
  TWI_MT_SLA_NACK           = TW_MT_SLA_NACK,             /*!< SLA+W has been transmitted; NACK received */
  TWI_MT_DATA_ACK           = TW_MT_DATA_ACK,             /*!< Data has been transmitted; ACK received */
  TWI_MT_DATA_NACK          = TW_MT_DATA_NACK,            /*!< Data has been transmitted; NACK received */
  TWI_MT_ARB_LOST           = TW_MT_ARB_LOST,             /*!< Arbitration lost in SLA+W or data transmission */
  TWI_MR_ARB_LOST           = TW_MR_ARB_LOST,             /*!< Arbitration lost in SLA+R or NACK received */
  TWI_MR_SLA_ACK            = TW_MR_SLA_ACK,              /*!< SLA+R has been transmitted; ACK received*/
  TWI_MR_SLA_NACK           = TW_MR_SLA_NACK,             /*!< SLA+R has been transmitted; NACK received */
  TWI_MR_DATA_ACK           = TW_MR_DATA_ACK,             /*!< Data has been received; ACK transmitted */
  TWI_MR_DATA_NACK          = TW_MR_DATA_NACK,            /*!< Data has been received; NACK transmitted */
  TWI_ST_SLA_ACK            = TW_ST_SLA_ACK,              /*!< Own SLA+R has been received; ACK transmitted */
  TWI_ST_ARB_LOST_SLA_ACK   = TW_ST_ARB_LOST_SLA_ACK,     /*!< SLA+R/W arbitration lost; own SLA+R received; ACK transmitted */
  TWI_ST_DATA_ACK           = TW_ST_DATA_ACK,             /*!< Data has been transmitted; ACK received */
  TWI_ST_DATA_NACK          = TW_ST_DATA_NACK,            /*!< Data has been transmitted; NACK received */
  TWI_ST_LAST_DATA          = TW_ST_LAST_DATA,            /*!< Data has been transmitted; ACK received */
  TWI_SR_SLA_ACK            = TW_SR_SLA_ACK,              /*!< Own SLA+W has been received; ACK transmitted */
  TWI_SR_ARB_LOST_SLA_ACK   = TW_SR_ARB_LOST_SLA_ACK,     /*!< SLA+R/W arbitration lost; own SLA+W received; ACK transmitted */
  TWI_SR_GCALL_ACK          = TW_SR_GCALL_ACK,            /*!< General call address received; ACK transmitted */
  TWI_SR_ARB_LOST_GCALL_ACK = TW_SR_ARB_LOST_GCALL_ACK,   /*!< SLA+R/W artibtration lost; global call address received; ACK transmitted */
  TWI_SR_DATA_ACK           = TW_SR_DATA_ACK,             /*!< Addressed with own SLA+W; Data received; ACK transmitted */
  TWI_SR_DATA_NACK          = TW_SR_DATA_NACK,            /*!< Addressed with own SLA+W; Data received; NACK transmitted */
  TWI_SR_GCALL_DATA_ACK     = TW_SR_GCALL_DATA_ACK,       /*!< Addressed with General Call address; Data received; ACK transmitted */
  TWI_SR_GCALL_DATA_NACK    = TW_SR_GCALL_DATA_NACK,      /*!< Addressed with General Call address; Data received; NACK transmitted */
  TWI_SR_STOP               = TW_SR_STOP,                 /*!< A Stop or Repeated Start condition has been received while addressed */
  TWI_NO_INFO               = TW_NO_INFO,                 /*!< No relevent status code */
  TWI_BUS_ERROR             = TW_BUS_ERROR,               /*!< Illegal start or stop condition */
  TWI_NOT_INIT              = 0xFD,                       /*!< TWI master not initializd */
  TWI_NO_WAIT               = 0xFE,                       /*!< TWI master posted write issued */
  TWI_TIMEDOUT              = 0xFF                        /*!< TWI timed out waiting for valid condition to continue. */
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
 * Will issue STOP condition [default] | Will issue Start / Repeated Start Condition 
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
 * @note Respected values returned from this function depend on the value of 
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

/**
 * @brief Structure used to initialize the TWI master.
 */
typedef struct
{
  F_SCL_FREQ F_scl_kHz;                   /*!< The SCL frequency in kHz */
  uint8_t  timeout;                       /*!< Timeout counter, will loop this many times sleeping for #DELAY_US microseconds until tested condition is met. */
  TWI_MASTER_COMPLETE complete_callback;  /*!< ::TWI_MASTER_COMPLETE */
  TWI_MASTER_NACK nack_callback;          /*!< ::TWI_MASTER_NACK */
} TWI_INIT;

/**
 * @brief Structure used to initialize the TWI master. This differs from
 *        ::TWI_INIT in that the value for TWBR and TWSR[1:0] is defined
 *        instead of the SCL frequency.
 */
typedef struct
{
  uint8_t twi_baud;                       /*!< The baud rate setting for TWBR */
  uint8_t timeout;                        /*!< Timeout counter, will loop this many times sleeping for #DELAY_US microseconds until tested condition is met. */ 
  TWI_PRESCALER prescaler;                /*!< The prescaler value to put for TWSR[1:0] */
  TWI_MASTER_COMPLETE complete_callback;  /*!< ::TWI_MASTER_COMPLETE */
  TWI_MASTER_NACK nack_callback;          /*!< ::TWI_MASTER_NACK */
} TWI_INIT2;

/**
 * @brief Structure used in master TX and RX operations.
 */
typedef struct
{
  uint8_t* data;          /*!< The buffer used to send or receive data. */
  uint8_t  data_sz;       /*!< Number of bytes available in TWI_MASTER_RW::data */
  uint8_t  no_start;      /*!< Don't issue a start condition. This should be set to 1 if a slave mode issued a Start Condition. */
  uint8_t  posted_write;  /*!< Complete a posted write, do not wait for all bytes to be issued to slave. */
} TWI_MASTER_RW;

/**
 * @brief The TWI slave callback configuration.
 */
typedef struct
{
  TWI_SLAVE_SLA       sla_callback;           /*!< ::TWI_SLAVE_SLA. Not required, default action will occur if not defined. */
  TWI_SLAVE_RX        rx_callback;            /*!< ::TWI_SLAVE_RX. Recommended, a NACK will be issued if not defined. */
  TWI_SLAVE_STOP      stop_callback;          /*!< ::TWI_SLAVE_STOP. Not required, default action will occur if not defined. */
  TWI_SLAVE_TX        tx_callback;            /*!< ::TWI_SLAVE_TX. Required. */
  TWI_SLAVE_LAST_DATA last_data_callback;     /*!< ::TWI_SLAVE_LAST_DATA. Not required, default action will occur if not defined. */
} TWI_SLAVE_CALLBACKS;

/**
 * @brief Initialize the TWI master.
 * @param init The initialization structure
 * @return TWI_OK
 */
TWI_STATUS twi_master(TWI_INIT* init);

/**
 * @brief Initialize the TWI master.
 * @param init The initialization structure
 * @return TWI_OK
 */
TWI_STATUS twi_master2(TWI_INIT2* init);

/**
 * @brief Issue a TWI stop condition
 * @return TWI_OK or TWI_TIMEDOUT
 */
TWI_STATUS twi_stop();

/**
 * @brief Disable TWI.
 * @return TWI_OK
 */
TWI_STATUS twi_disable();

/**
 * @brief Start a transmission of data to a slave.
 * @param address The slave or general address
 * @param tx_data Populated ::TWI_MASTER_RW structure with data, data size, etc.
 * @return
 * TWI_STATUS | Description
 * ---|---
 * TWI_NOT_INIT | TWI master not initialized. Call ::twi_master.
 * TWI_TIMEDOUT | A timeout occurred waiting for correct condition
 * TWI_NO_WAIT | Caller requested a posted write
 * TWI_MT_* | See ::TWI_STATUS
 */
TWI_STATUS twi_master_tx(uint8_t address, TWI_MASTER_RW* tx_data);

/**
 * @brief Start a reception of data from a slave.
 * @param address The slave address
 * @param rx_data Populated ::TWI_MASTER_RW structure with data, and data size.
 * @return
 * TWI_STATUS | Description
 * ---|---
 * TWI_NOT_INIT | TWI master not initialized. Call ::twi_master.
 * TWI_TIMEDOUT | A timeout occurred waiting for correct condition
 * TWI_MR_* | See ::TWI_STATUS
 */
TWI_STATUS twi_master_rx(uint8_t address, TWI_MASTER_RW* rx_data);

/**
 * @brief Initialize the TWI slave.
 * @param address The slave address shifted with optional general call bit set.
 *                Use #TWI_SLAVE_GENERAL_CALL or #TWI_SLAVE_NO_GENERAL_CALL
 * @param address_mask The address mask if slave is to respond to multiple
 *                     addresses. Set to 0 for single address. This function
                       will perform the required shift to write to TWAMR.
 * @param callbacks The callbacks for the slave.
 * @return TWI_OK
 */
TWI_STATUS twi_slave(uint8_t address, uint8_t address_mask, TWI_SLAVE_CALLBACKS* callbacks);

#endif // __TWI_H__
