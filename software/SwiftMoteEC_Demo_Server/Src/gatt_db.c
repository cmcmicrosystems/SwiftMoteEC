/******************** (C) COPYRIGHT 2019 STMicroelectronics ********************
* File Name          : gatt_db.c
* Author             : BlueNRG-LP database file for Sensor demo
* Version            : V1.0.0
* Date               : 02-April-2019
* Description        : Functions to build GATT DB and handle GATT events.
* Modification			 : Code is modified by CMC Microsystems in Sept. 2023
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ble_const.h" 
#include "bluenrg_lp_stack.h"
#include "clock.h" 
#include "gp_timer.h" 
#include "gatt_db.h"
#include "osal.h"
#include "SensorDemo_config.h"
#include "gatt_profile.h"
#include "gap_profile.h"
#include "OTA_btl.h" 


#include "bluenrg_lp_mote_config.h" 
#include "SqrWaveVoltammetry.h"
#include "ad5941main.h"

#ifndef DEBUG
#define DEBUG 0
#endif


#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//#define APPBUFF_SIZE 1200
//uint32_t AppBuff[APPBUFF_SIZE];


#define VOLTAMMOGRAM_SERVICE_UUID				0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x4b,0xcf,0x80,0x1b,0x0a,0x35	
#define VOLTAMMOGRAM_UUID								0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x4b,0xcf,0x80,0x1b,0x0a,0x34
#define RUN_UUID												0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x4b,0xcf,0x80,0x1b,0x0a,0x33
#define CONTINUOUS_RUN_UUID							0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x4b,0xcf,0x80,0x1b,0x0a,0x32
#define STOP_UUID												0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x4b,0xcf,0x80,0x1b,0x0a,0x31

uint16_t voltammogramServHandle;  //service handler
uint16_t voltammogramCharHandle1;  //sensor data handler
uint16_t voltammogramCharHandle2;   // run CMD handler
uint16_t voltammogramCharHandle3;   // continuous run CMD handle
uint16_t voltammogramCharHandle4;   // stop CMD handle


extern uint16_t connection_handle;



/* Client Configuration Characteristics Descriptor Definition: voltammogram characteristic*/
BLE_GATT_SRV_CCCD_DECLARE(voltamm,
													NUM_LINKS,
													BLE_GATT_SRV_CCCD_PERM_DEFAULT,
													BLE_GATT_SRV_OP_MODIFIED_EVT_ENABLE_FLAG);

#if PARAMS_NOTIFY
BLE_GATT_SRV_CCCD_DECLARE(params,
													NUM_LINKS,
													BLE_GATT_SRV_CCCD_PERM_DEFAULT,
													BLE_GATT_SRV_OP_MODIFIED_EVT_ENABLE_FLAG);

#endif

/* Acceleration Service Characteristics Definition */
#if DUMMY_DATA_NOTIFY
static ble_gatt_chr_def_t volt_chars[] = {

//		
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_NOTIFY | BLE_GATT_SRV_CHAR_PROP_READ,
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(VOLTAMMOGRAM_UUID),
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(voltamm),
            .descr_count = 1U,
        },
    },
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_NOTIFY  | BLE_GATT_SRV_CHAR_PROP_WRITE, // notification for confirmation that write was successful
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(VOLTAMMOGRAM_PARAMS_UUID),
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(params),
            .descr_count = 1U,
        },
    },

};
#endif

#if SENSOR_DATA
static ble_gatt_chr_def_t volt_chars[] = {

//		/* Voltammagram characteristic */
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_INDICATE | BLE_GATT_SRV_CHAR_PROP_READ,
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(VOLTAMMOGRAM_UUID),
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(voltamm),
            .descr_count = 1U,
        },
    },
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_INDICATE  | BLE_GATT_SRV_CHAR_PROP_WRITE_NO_RESP| BLE_GATT_SRV_CHAR_PROP_WRITE, // notification for confirmation that write was successful
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(RUN_UUID),
#if PARAMS_NOTIFY
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(params),
            .descr_count = 1U,
        },
#endif
    },
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_INDICATE  | BLE_GATT_SRV_CHAR_PROP_WRITE_NO_RESP| BLE_GATT_SRV_CHAR_PROP_WRITE, // notification for confirmation that write was successful
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(CONTINUOUS_RUN_UUID),
#if PARAMS_NOTIFY
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(params),
            .descr_count = 1U,
        },
#endif
    },
		{
        .properties = BLE_GATT_SRV_CHAR_PROP_INDICATE  | BLE_GATT_SRV_CHAR_PROP_WRITE_NO_RESP| BLE_GATT_SRV_CHAR_PROP_WRITE, // notification for confirmation that write was successful
        .permissions = BLE_GATT_SRV_PERM_NONE,
        .min_key_size = BLE_GATT_SRV_MIN_ENCRY_KEY_SIZE,
        .uuid = BLE_UUID_INIT_128(STOP_UUID),
#if PARAMS_NOTIFY
        .descrs = {
            .descrs_p = &BLE_GATT_SRV_CCCD_DEF_NAME(params),
            .descr_count = 1U,
        },
#endif
    },

};
#endif

/* Voltammogramm service definition */
static ble_gatt_srv_def_t volt_service = {
   .type = BLE_GATT_SRV_PRIMARY_SRV_TYPE,
   .uuid = BLE_UUID_INIT_128(VOLTAMMOGRAM_SERVICE_UUID),
   .chrs = {
       .chrs_p = volt_chars,
       .chr_count = 4U,
   },
};



tBleStatus Add_Volt_Service(void)
{
  tBleStatus ret;

  ret = aci_gatt_srv_add_service(&volt_service);
  if (ret != BLE_STATUS_SUCCESS)
  {
    goto fail;
  }
  voltammogramServHandle = aci_gatt_srv_get_service_handle(&volt_service);
	voltammogramCharHandle1 = aci_gatt_srv_get_char_decl_handle(&volt_chars[0]);  //sensor data
	voltammogramCharHandle2 = aci_gatt_srv_get_char_decl_handle(&volt_chars[1]);   // run cmd
	voltammogramCharHandle3 = aci_gatt_srv_get_char_decl_handle(&volt_chars[2]);  //continuous run cmd
	voltammogramCharHandle4 = aci_gatt_srv_get_char_decl_handle(&volt_chars[3]);  //stop cmd
  PRINTF("Service VOLT added. Handle 0x%04X , 0x%04X , 0x%04X  \n",
         voltammogramServHandle, voltammogramCharHandle1, voltammogramCharHandle2);

  return BLE_STATUS_SUCCESS; 

 fail:
  PRINTF("Error while adding VOLTAMMOGRAM service.\n");
  return BLE_STATUS_ERROR ;
}

/* The update_char function does the following: 
* a). pack sensor data into a transmission buffer "buff", in uint16_t and little endian format 
* b). notify GATT client (the program that receives the BLE data packets on a PC or other device)
* c). re-transmit any data packet in the case either aci_gatt_srv_notify fails or did not receive acknowledgement from the client
* Description: It transmits loop_n data packets with a for loop that is indexted by i.Each data packets size is BUFF_SIZE bytes except the last one 
* that contains last_packet_s bytes. When there is no interruption, for example, BLE disconnection or any other cause of re-transmission of a data packet,
* the for loop will be start from 0 (the initial value of loop_start) to loop_n; in the case, there is re-transmission takes place,the for loop will resume
* from loop_start that indicates where it is left from the interruption. Inside of the for loop indexed by i, there is another for loop indexed by j that is
* used to pack each data packet into the transmission buffer " buff".
*/


#define BUFF_SIZE 244
uint16_t  data_coursor = 0; //used to tell how many data being transferred so that tx_ready can be reset
uint8_t buff_to_retransmit[BUFF_SIZE] = {0}; //buffer used to hold data packet that need to be re-transmitted
uint8_t to_retransmit = 0; // re-transmission flag
uint16_t  loop_start = 0; //deal with where the loop starts when there is re-transmission. 
extern uint32_t TotalCnt;  //the total data counts generated by AD5941, need to be reset to 0 after each voltammogram is transmitted
extern int connected; // used to check connection status 
extern int indicat_Ack;  //ackwonledgement sent by client upon receiving the indication sent by aci_gatt_srv_notify
extern uint16_t endseq; // end of sequence of generating a complete voltammogram, need to be reset to 0 after each voltammogram is transmitted. 
extern uint8_t tx_ready; // indicating a complete voltammogram data has been read off AD5941 and it is ready for BLE to transmit it
extern uint8_t ramp_init_done; // a global flag indicate that the square wave configuration is done. it need to be reset after a voltammogram is generated


#if SENSOR_DATA
uint8_t update_char(int32_t * const pData, uint32_t *pDataCount)
{
	uint8_t buff[BUFF_SIZE] = {0}; //transmission buffer used in aci_gatt_srv_notify
	tBleStatus ret;	
	uint32_t i, datacount;
	
  volatile uint16_t  last_packet_s;		
	volatile uint16_t  loop_n;
	volatile uint16_t  buff_datacount;

	datacount = *pDataCount;
	
		last_packet_s = (datacount*2+4) % BUFF_SIZE; //the total data need to be transmitted is voltammogram data + 4 bytes of meta data
		buff_datacount = BUFF_SIZE/2;
	
	  if (last_packet_s == 0)
				loop_n = datacount*2 / BUFF_SIZE;
		else
			  loop_n = (datacount*2 / BUFF_SIZE)+1;
		
		
	if (to_retransmit==1)
		{

			if (data_coursor == datacount)
			ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle1 + 1, 0x02, last_packet_s, buff_to_retransmit);
			else
			ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle1 + 1, 0x02, BUFF_SIZE, buff_to_retransmit);
			
			if (ret == 0x00 && indicat_Ack)
			{
				to_retransmit=0;
				memset(buff_to_retransmit, 0, BUFF_SIZE);
				if (data_coursor == datacount)  // when the last data packet was re-transmitted successfully
				{
		       tx_ready = 0;
					 data_coursor = 0;
					 loop_start = 0;
					 TotalCnt = 0;
					
					 return 0;
				}	
			}else
			{
				return 1; // try again later
		  }
	}
	
/*the following couple of lines of code is to flush the connection at the beginning of data packet transmission to remove
*	the duplicated first data packet issue as a temporary solution. it should be removed once a fixed is provided.
*/	
	
	uint8_t buff_val[2];
	buff_val[0] = 1;
	buff_val[1] = 0;
	ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle1 + 1, 0x02, 2, buff_val);
																																															
						
/* the main loop to transmit the sensor data by packing them into buff and notify the client*/								
	  	
		for (uint8_t i = loop_start; i < loop_n; i++)
			{			 
					loop_start++;
				
				//the first data packet contains the info of number of data points in one voltammogram 
				//and also inclide indicators (**) to indicate it is the first data packet of the voltammogram
				  if (i==0)  
					{
						HOST_TO_LE_16(buff, '*');
						HOST_TO_LE_16(buff+1, '*');
						HOST_TO_LE_16(buff+2, datacount&0xffff);
					  for (int j = 0; j < buff_datacount-2; j++) //4 bytes (or space for two data points)of the first data packet are used for meta info
						{
							HOST_TO_LE_16(buff+4+j*2, pData[j]&0xffff); 
							data_coursor++;
						}						
												
					}else if (i==(loop_n-1))
					{
						for (int j = 0; j < last_packet_s/2; j++)
							{
								HOST_TO_LE_16(buff+j*2, pData[data_coursor]&0xffff);
								data_coursor++;
							}
							
					}else					
					{
					
						for (int j = 0; j < buff_datacount; j++)
							{
								HOST_TO_LE_16(buff+j*2, pData[data_coursor]&0xffff); 
								data_coursor++;
							}			
					}
					  
						indicat_Ack = FALSE;
						ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle1 + 1, 0x02, BUFF_SIZE, buff);
					
					if((ret!=0) || (!indicat_Ack))   
						{	
							break;
						}
						memset(buff, 0, BUFF_SIZE);
										
			}	
			
			
	
			if((ret!=0) || (!indicat_Ack))
			{		
			
				memcpy(buff_to_retransmit, buff,BUFF_SIZE); // save failed packet to retransmit later
				memset(buff, 0, BUFF_SIZE);
				//BSP_LED_On(BSP_LED);
				to_retransmit=1;
				return 1;	
		}		
		
	 if ((data_coursor == datacount) && ret==0 && indicat_Ack) // after the last data packet was transmitted
		
	 {
				 tx_ready = 0;
		     data_coursor = 0;
		     loop_start = 0;
		     TotalCnt = 0;
	 }
							
	return 0;		
}
#endif





#if DUMMY_DATA_NOTIFY
void Dummy_Data_Notify(void)
{  
  uint8_t val;
	//uint16_t val;
  tBleStatus ret;
	uint8_t dummy_data[] ={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

  val = 0x02;
  int buff_size;		

   buff_size = sizeof(dummy_data);

   //ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle2 + 1, 0, 0x01, &val);
	 ret = aci_gatt_srv_notify(connection_handle, voltammogramCharHandle2 + 1, 0, buff_size, dummy_data);
  if (!ret)
					PRINTF("notification failed \n ");
				else
					PRINTF("notification good \n ");
}



void aci_gatt_srv_write_event(uint16_t Connection_Handle, uint8_t Resp_Needed, uint16_t Attribute_Handle, uint16_t Data_Length, uint8_t Data[])
{
    uint8_t att_error = BLE_ATT_ERR_NONE;
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  OTA_Write_Request_CB(Connection_Handle, Attribute_Handle, Data_Length, Data); 
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
    if (Resp_Needed == 1U)
    {
			aci_gatt_srv_resp(Connection_Handle, 0, att_error, 0, NULL); 
    }
}

void aci_gatt_srv_read_event(uint16_t Connection_Handle, uint16_t Attribute_Handle, uint16_t Data_Offset)
{
    uint8_t att_err;
    uint8_t buff[6], *data_p;
    uint16_t data_len;
    att_err = BLE_ATT_ERR_NONE;
	
//		printf("Read_event: %02X , %02X ,%02X \r\n", Connection_Handle, Attribute_Handle, Data_Offset);
    if(Attribute_Handle == voltammogramCharHandle1+1)
    {
			  //float f = 3.7;
				//float* pf = &f;
			
        HOST_TO_LE_32(buff, *((uint32_t*) pf));
				data_len = 4;
				data_p = buff;
		}
		else if(Attribute_Handle == voltammogramCharHandle2+1)
    {
        HOST_TO_LE_16(buff, 0x5678); // Demo of sending any value upon request.
				data_len = 2;
				data_p = buff;
		}

    
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT 
    OTA_Read_Char(Connection_Handle, Attribute_Handle,Data_Offset);
#endif 

    aci_gatt_srv_resp(Connection_Handle, Attribute_Handle, att_err, data_len, data_p);
}

#endif


void ad5941_app_init(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
	uint16_t     SVolt;
	uint16_t     FVolt;
	uint16_t     Volt_step;
	uint16_t     Gain;
	uint16_t     Rtia;
	uint16_t     Freq;
	uint16_t     Amplitude;
	


			SVolt = att_data[0] | (att_data[1] << 8);
			FVolt = att_data[2] | (att_data[3] << 8);
			Volt_step = att_data[4] | (att_data[5] << 8);
	    Gain = att_data[6] | (att_data[7] << 8);
	    Rtia = att_data[8] | (att_data[9] << 8);
	    Freq = att_data[10] | (att_data[11] << 8);
	    Amplitude = att_data[12] | (att_data[13] << 8);

	    AD5941RampStructInit(&SVolt, &FVolt, &Volt_step, &Gain, &Rtia, &Freq, &Amplitude);


#if POWER_SAVE
			HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_NOTIMER, wakeupIO, &stopLevel);   // put BlueNRG-LP into sleep
#endif
  
}



uint8_t run_once_f = 0;
uint8_t cont_run_f = 0;
uint8_t stop_f = 0;
extern uint8_t ramp_init_done;
void aci_gatt_srv_write_event(uint16_t Connection_Handle, uint8_t Resp_Needed, uint16_t Attribute_Handle, uint16_t Data_Length, uint8_t Data[])
{
    uint8_t att_error = BLE_ATT_ERR_NONE;

	if(Attribute_Handle == voltammogramCharHandle2 + 1)   //run once
  {
    ad5941_app_init(Attribute_Handle, Data_Length, Data);
		run_once_f = 1;
		cont_run_f = 0;
		stop_f = 0;
  }
	
	if(Attribute_Handle == voltammogramCharHandle3 + 1)  //run continously
  {
    ad5941_app_init(Attribute_Handle, Data_Length, Data);
		run_once_f = 0;
		cont_run_f = 1;
		stop_f = 0;
  }
	
	if(Attribute_Handle == voltammogramCharHandle4 + 1)  //stop continuous run
  {
 		run_once_f = 0;
		cont_run_f = 0;
		stop_f = 1;
		ramp_init_done = 0;
  }
  
    if (Resp_Needed == 1U)
    {
        aci_gatt_srv_resp(Connection_Handle, 0, att_error, 0, NULL);
    }
}

void aci_gatt_srv_read_event(uint16_t Connection_Handle, uint16_t Attribute_Handle, uint16_t Data_Offset)
{
  uint8_t att_err;
  //BSP_LED_On(BSP_LED);
  att_err = BLE_ATT_ERR_NONE;
  if(Attribute_Handle == voltammogramCharHandle2 + 1)
  {
    aci_gatt_srv_resp(Connection_Handle, Attribute_Handle, att_err, 0, NULL);
  }
}
