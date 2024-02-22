#include "bluenrg_lp_stack.h"
#ifndef _GATT_DB_H_
#define _GATT_DB_H_

#include "bluenrg_lp_mote_config.h" 


#ifndef SENSOR_DATA
#define SENSOR_DATA 1
#endif

#ifndef POWER_SAVE
#define POWER_SAVE 0
#endif

#ifndef RE_TRANSMIT
#define RE_TRANSMIT 0
#endif

#ifndef DUMMY_DATA_NOTIFY
#define DUMMY_DATA_NOTIFY 0
#endif


/** 
 * @brief Number of application services
 */
#define NUMBER_OF_APPLICATION_SERVICES (2)

tBleStatus Add_Volt_Service(void);

#if RE_TRANSMIT
//uint8_t Volt_Update(uint16_t * const pData, uint32_t *pDataCount);
uint8_t Volt_Update(int32_t * const pData, uint32_t *pDataCount);
#endif

#if SENSOR_DATA
uint8_t update_char(int32_t * const pData, uint32_t *pDataCount);
#endif


#if DUMMY_DATA_NOTIFY
void Dummy_Data_Notify(void);
#endif

#endif /* _GATT_DB_H_ */
