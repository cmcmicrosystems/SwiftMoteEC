/******************** (C) COPYRIGHT 2019 STMicroelectronics ********************
 * File Name          : sensor.c
 * Author             : AMS - RF Application team
 * Version            : V2.0.0
 * Date               : 26-February-2019
 * Description        : Sensor init and sensor state machines
 * Modification				: Code is modified by CMC Microsystems in Sept. 2023
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "ble_const.h" 
#include "bluenrg_lp_stack.h"
#include "OTA_btl.h" 

#include "gatt_db.h"
#include "rf_driver_hal_vtimer.h"
#include "gap_profile.h"
#include "clock.h"
#include "gp_timer.h"
#include "SensorDemo_config.h"

#include "ad5941main.h"
#include "ad5941.h"
#include "SqrWaveVoltammetry.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif



#define UPDATE_CONN_PARAM 1 //0
#define ADV_INTERVAL_MIN_MS  500  //min. adv. interval is 0.2s
#define ADV_INTERVAL_MAX_MS  500

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint8_t set_connectable = 1;
uint16_t connection_handle = 0;
uint8_t connInfo[20];
int connected = FALSE;
int indicat_Ack = FALSE;


#if UPDATE_CONN_PARAM
#define UPDATE_TIMER 2 //TBR
static VTIMER_HandleType l2cap_TimerHandle;
int l2cap_request_sent = FALSE;
static uint8_t l2cap_req_timer_expired = FALSE; 
#endif

  
/* Private function prototypes -----------------------------------------------*/

static Advertising_Set_Parameters_t Advertising_Set_Parameters[1];

/* Private functions ---------------------------------------------------------*/



/*******************************************************************************
 * Function Name  : Sensor_DeviceInit.
 * Description    : Init the device sensors.
 * Input          : None.
 * Return         : Status.
 *******************************************************************************/
uint8_t  Sensor_DeviceInit()
{
  uint8_t ret;
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  uint8_t device_name[] = {'B', 'l', 'u', 'e', 'N', 'R', 'G'};
  static uint8_t adv_data[] = {0x02,AD_TYPE_FLAGS, FLAG_BIT_LE_GENERAL_DISCOVERABLE_MODE|FLAG_BIT_BR_EDR_NOT_SUPPORTED,
                              8, AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G'};

  /* Set the TX power 0 dBm */
  //aci_hal_set_tx_power_level(0, 24);
		aci_hal_set_tx_power_level(0, 15);
  
  /* GATT Init */
  ret = aci_gatt_srv_init(); //Adds the GATT service to the database.
  if (ret != BLE_STATUS_SUCCESS) {
//    PRINTF("aci_gatt_srv_init() failed: 0x%02x\r\n", ret);
    return ret;
  }
  
  /* GAP Init*/
	
  ret = aci_gap_init(GAP_PERIPHERAL_ROLE, 0, 0x07, STATIC_RANDOM_ADDR, &service_handle, &dev_name_char_handle, &appearance_char_handle); 
  if (ret != BLE_STATUS_SUCCESS) {
//    PRINTF("aci_gap_init() failed: 0x%02x\r\n", ret);
    return ret;
  }
 
  /* Update device name */
  Gap_profile_set_dev_name(0, sizeof(device_name), device_name);
  
	
	/*Set the authentication requirements for the device*/
  ret = aci_gap_set_authentication_requirement(BONDING,
                                               MITM_PROTECTION_REQUIRED,
                                               SC_IS_NOT_SUPPORTED,
                                               KEYPRESS_IS_NOT_SUPPORTED,
                                               7, 
                                               16,
                                               USE_FIXED_PIN_FOR_PAIRING,
                                               123456);
  if(ret != BLE_STATUS_SUCCESS) {
//    PRINTF("aci_gap_set_authentication_requirement()failed: 0x%02x\r\n", ret);
    return ret;
  }

  ret = aci_gap_set_advertising_configuration( 0, GAP_MODE_GENERAL_DISCOVERABLE,
                                              ADV_PROP_CONNECTABLE|ADV_PROP_SCANNABLE|ADV_PROP_LEGACY,
                                              (ADV_INTERVAL_MIN_MS*1000)/625, (ADV_INTERVAL_MAX_MS*1000)/625, 
                                              ADV_CH_ALL,
                                              0,NULL,
                                              ADV_NO_WHITE_LIST_USE,
                                              0, /* 0 dBm */
                                              LE_2M_PHY, /* Primary advertising PHY */
                                              0, /* 0 skips */
                                              LE_2M_PHY, /* Secondary advertising PHY. Not used with legacy advertising. */
                                              0, /* SID */
                                              0 /* No scan request notifications */);
	
  PRINTF("Advertising configuration 0x%02X\n", ret);
  
	/*set the data in advertising PDUs*/
  ret = aci_gap_set_advertising_data( 0, ADV_COMPLETE_DATA, sizeof(adv_data), adv_data);
  
  /* Add Volt service and Characteristics */
  ret = Add_Volt_Service();
  if(ret == BLE_STATUS_SUCCESS) {
    //PRINTF("Voltammogram service added successfully.\n");
  } else {
  //  PRINTF("Error while adding Voltammogram service: 0x%02x\r\n", ret);
    return ret;
  }


#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT     
  ret = OTA_Add_Btl_Service();
  if(ret == BLE_STATUS_SUCCESS)
//    PRINTF("OTA service added successfully.\n");
  else
    //PRINTF("Error while adding OTA service.\n");
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
  

  return BLE_STATUS_SUCCESS;
}


#if UPDATE_CONN_PARAM
void l2cap_UpdateTimeoutCB(void *param)
{
  l2cap_req_timer_expired = TRUE;
}
#endif

/*******************************************************************************
 * Function Name  : Set_DeviceConnectable.
 * Description    : Puts the device in connectable mode.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Set_DeviceConnectable(void)
{  
  uint8_t ret;

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  ret = aci_gap_set_scan_response_data(0,18,BTLServiceUUID4Scan);
  if(ret != BLE_STATUS_SUCCESS)
  {
    PRINTF("aci_gap_set_scan_response_data() failed: 0x%02x\r\n",ret);
 
  }
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 

  Advertising_Set_Parameters[0].Advertising_Handle = 0;
  Advertising_Set_Parameters[0].Duration = 0;
  Advertising_Set_Parameters[0].Max_Extended_Advertising_Events = 0;
  
  ret = aci_gap_set_advertising_enable(ENABLE, 1, Advertising_Set_Parameters); 
  
  if(ret != BLE_STATUS_SUCCESS)
  {
    //PRINTF("aci_gap_set_advertising_enable() failed: 0x%02x\r\n",ret);
     
  }
  else
    PRINTF("aci_gap_set_advertising_enable() --> SUCCESS\r\n");
}

/*******************************************************************************
 * Function Name  : APP_Tick.
 * Description    : Sensor Demo state machine. sensor data generation first, then check connection, then transmit data
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/

#if SENSOR_DATA
uint8_t tx_ready = 0;  //indicates wether or not data for one voltammegram has been transmitted.
uint32_t temp = 0; //number of data generated, obtain value in AppSWVISR.
extern uint32_t AppBuff[APPBUFF_SIZE];
extern uint16_t endseq;
extern uint8_t ramp_init_done;

extern uint8_t run_once_f;
extern uint8_t cont_run_f;
extern uint8_t stop_f;

void app_read_data(void)
{
	int loop = 1;
	
		AppSWVInit(AppBuff, APPBUFF_SIZE);    
    AppSWVCtrl(APPCTRL_START, 0);
 		
	while(1)
	 {
			if(AD5941_GetMCUIntFlag())
				{
   
					AD5941_ClrMCUIntFlag();
					temp = APPBUFF_SIZE;
					AppSWVISR(AppBuff, &temp);
									 				
				}
				if(endseq ==1)
					{
						endseq = 0;
						tx_ready = 1;
						break;
					}				
		 }	 
}


void APP_Tick()
{
    uint32_t ret_volt;
	
#if UPDATE_CONN_PARAM    
  uint8_t ret = 0; 
#endif 
  /* Make the device discoverable*/
 		 
	if(set_connectable) {
		//BSP_LED_On(BSP_LED);
    Set_DeviceConnectable();
    set_connectable = FALSE;
  }

#if UPDATE_CONN_PARAM      

  if(connected && !l2cap_request_sent && l2cap_req_timer_expired){
		
		ret = aci_l2cap_connection_parameter_update_req(connection_handle, 0x0006, 0x0C80, 0x01F3, 3200);
    l2cap_request_sent = TRUE;
  }
#endif	 

	// the program will keep running and scan and read and transmit data until it is a run_once command or stop command that set the run_f to false
//	so that it won't read data and therefore will not set tx_ready flag
	if ((!tx_ready)&& ramp_init_done) 
	{
		app_read_data();
		if (run_once_f)
		{
			ramp_init_done = 0;
		}
	}
		
	
	if((tx_ready==1) && connected)
	  {    
		   ret_volt = update_char((int32_t *)AppBuff,&temp);
	   }
			 
#if POWER_SAVE
	HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_NOTIMER, wakeupIO, &stopLevel);
#endif


}
#endif


#if DUMMY_DATA_NOTIFY
void APP_Tick()
{
	uint32_t ret_volt;
	
#if UPDATE_CONN_PARAM    
  uint8_t ret = 0; 
#endif 
  /* Make the device discoverable */
 
		 
	if(set_connectable) {
    Set_DeviceConnectable();
    set_connectable = FALSE;
  }

#if UPDATE_CONN_PARAM      

  if(connected && !l2cap_request_sent && l2cap_req_timer_expired){
		
		//ret = aci_l2cap_connection_parameter_update_req(connection_handle, 0x0006, 0x0C80, 0x01F3, 3200);
		ret = aci_l2cap_connection_parameter_update_req(connection_handle, 0x0320, 0x0C80, 0x01F3, 3200); //conn min interval = 1s, max is 4s
		//ret = aci_l2cap_connection_parameter_update_req(connection_handle, 0x0C80, 0x0C80, 0x01F3, 3200);
    l2cap_request_sent = TRUE;
  }
#endif	 

  if(connected)
	{    
		//BSP_LED_On(BSP_LED);
		Dummy_Data_Notify();
	}
	
}
#endif

/* ***************** BlueNRG-LP Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
#if UPDATE_CONN_PARAM  
  uint8_t ret; 
#endif
  
  if(Status != BLE_STATUS_SUCCESS)
    return;
  
  connected = TRUE;
	//BSP_LED_On(BSP_LED);
  connection_handle = Connection_Handle;
  
#if UPDATE_CONN_PARAM    
  l2cap_request_sent = FALSE;
  
   /* Start the l2cap Timer */
  l2cap_TimerHandle.callback = l2cap_UpdateTimeoutCB;  
  ret = HAL_VTIMER_StartTimerMs(&l2cap_TimerHandle, CLOCK_SECOND*2);
  if (ret != BLE_STATUS_SUCCESS) {
    //PRINTF("HAL_VTIMER_StartTimerMs(l2cap) failed; 0x%02x\r\n", ret);
  } else {
    l2cap_req_timer_expired = FALSE;
  }
#endif
HAL_Delay(2); // to test if the duplicated first data packet after the connection was just established caused by unstable connection status at the very beginning
	
}/* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_le_enhanced_connection_complete_event.
 * Description    : This event indicates that a new connection has been created
 * Input          : See file bluenrg_lp_events.h
 * Output         : See file bluenrg_lp_events.h
 * Return         : See file bluenrg_lp_events.h
 *******************************************************************************/
void hci_le_enhanced_connection_complete_event(uint8_t Status,
                                               uint16_t Connection_Handle,
                                               uint8_t Role,
                                               uint8_t Peer_Address_Type,
                                               uint8_t Peer_Address[6],
                                               uint8_t Local_Resolvable_Private_Address[6],
                                               uint8_t Peer_Resolvable_Private_Address[6],
                                               uint16_t Conn_Interval,
                                               uint16_t Conn_Latency,
                                               uint16_t Supervision_Timeout,
                                               uint8_t Master_Clock_Accuracy)
{
  
  hci_le_connection_complete_event(Status,
                                   Connection_Handle,
                                   Role,
                                   Peer_Address_Type,
                                   Peer_Address,
                                   Conn_Interval,
                                   Conn_Latency,
                                   Supervision_Timeout,
                                   Master_Clock_Accuracy);
}

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event occurs when a connection is terminated.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
  connected = FALSE;
	//BSP_LED_Off(BSP_LED);
  /* Make the device connectable again. */
  set_connectable = TRUE;
  connection_handle =0;
  
   
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  OTA_terminate_connection();
#endif 
}/* end hci_disconnection_complete_event() */


/*******************************************************************************
 * Function Name  : aci_gatt_srv_attribute_modified_event.
 * Description    : This event occurs when an attribute is modified.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_srv_attribute_modified_event(uint16_t Connection_Handle,
                                           uint16_t Attr_Handle,
                                           uint16_t Attr_Data_Length,
                                           uint8_t Attr_Data[])
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  OTA_Write_Request_CB(Connection_Handle, Attr_Handle, Attr_Data_Length, Attr_Data); 
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
}


void aci_gatt_srv_confirmation_event(	uint16_t 	Connection_Handle	)
{
  indicat_Ack = TRUE;
//	BSP_LED_On(BSP_LED);

}

void aci_hal_end_of_radio_activity_event(uint8_t Last_State,
                                         uint8_t Next_State,
                                         uint32_t Next_State_SysTime)
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  if (Next_State == 0x02) /* 0x02: Connection event slave */
  {
    OTA_Radio_Activity(Next_State_SysTime);  
  }
#endif 
}

#if UPDATE_CONN_PARAM

void aci_l2cap_connection_update_resp_event(uint16_t Connection_Handle,
                                            uint16_t Result)
{
   //PRINTF("aci_l2cap_connection_update_resp_event; 0x%02x\r\n", Result);
}

void aci_l2cap_command_reject_event(uint16_t Connection_Handle,
                                    uint8_t Identifier,
                                    uint16_t Reason,
                                    uint8_t Data_Length,
                                    uint8_t Data[])
{
  //PRINTF("aci_l2cap_command_reject_event; 0x%02x\r\n", Reason);
}
#endif 

void hci_le_connection_update_complete_event(uint8_t Status,
                                             uint16_t Connection_Handle,
                                             uint16_t Conn_Interval,
                                             uint16_t Conn_Latency,
                                             uint16_t Supervision_Timeout)
{
  //BSP_LED_On(BSP_LED);
	//PRINTF("hci_le_connection_update_complete_event; %d\r\n", Conn_Interval);
}

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
void aci_att_exchange_mtu_resp_event(uint16_t Connection_Handle,
                                     uint16_t Server_RX_MTU)
{
  OTA_att_exchange_mtu_resp_CB(Connection_Handle, Server_RX_MTU);
}

void hci_le_data_length_change_event(uint16_t Connection_Handle,
                                     uint16_t MaxTxOctets,
                                     uint16_t MaxTxTime,
                                     uint16_t MaxRxOctets,
                                     uint16_t MaxRxTime)
{
  OTA_data_length_change_CB(Connection_Handle);  
}
#endif
