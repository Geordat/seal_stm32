#include <string.h>

#include "st25ftm_common.h"
#include "st25ftm_config.h"
#include "stm32f7xx_hal.h"
#include "st25dv.h"
#include "ili9341_cube.h"
#include "stm32f7xx_hal_rcc.h"
//#include "common.h"

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include <string.h>

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#undef FAIL
#define FAIL 0

#undef PASS
#define PASS !FAIL

/* Exported constants --------------------------------------------------------*/
#define MAX_NDEF_MEM                 ((uint32_t)NFCTAG_64K_SIZE)
#define NFC_DEVICE_MAX_NDEFMEMORY    ((uint32_t)MAX_NDEF_MEM)

#define RESULTOK                     0x00
#define ERRORCODE_GENERIC            1
/* Exported functions ------------------------------------------------------- */
#define BSP_BUTTON_USER_IT_PRIORITY         0x0FU

#define ST25DX_DISCOVERY_TS_I2C_ADDRESS     0x82

#define ST25DX_DISCOVERY_INIT_CLK_GPO_RFD() __HAL_RCC_GPIOE_CLK_ENABLE( );
#define ST25DX_DISCOVERY_INIT_CLK_LPD_RFD() __HAL_RCC_GPIOA_CLK_ENABLE( );

#define BSP_NFCTAG_BUS_HANDLE               (hbus_i2c2)
#define BSP_NFCTAG_INSTANCE                 0U

#define BSP_GPO_EXTI                        EXTI15_10_IRQn
#define BSP_GPO_PRIORITY                    0U
#define BSP_GPO_EXTI_LINE                   (EXTI_LINE_15)

#ifndef USE_BSP_COM_FEATURE
#define USE_BSP_COM_FEATURE                 1U
#endif

#define USE_COM_LOG                         1U
//
//static CRC_HandleTypeDef hcrc;
extern CRC_HandleTypeDef hcrc;
static uint8_t FieldOnEvt;
static uint8_t FieldOffEvt;
/*extern*/ uint8_t GPO_Activated;
ST25FTM_MessageOwner_t mailboxStatus = ST25FTM_MESSAGE_EMPTY;
#if (ST25FTM_ENABLE_LOG != 0)
/* This buffer is used to store formatted text to transmit over UART */
char st25ftm_dbg_txt[512];
#endif

static void ManageGPO( void );
static int32_t InitITGPOMode( const uint16_t ITConfig );


/* Exported macro ------------------------------------------------------------*/
#define ST25_RETRY_NB     ((uint8_t) 15)
#define ST25_RETRY_DELAY  ((uint8_t) 40)

/**
  * @brief Iterate ST25DV command depending on the command return status.
  * @param cmd A ST25DV function returning a NFCTAG_StatusTypeDef status.
  */
#define ST25_RETRY(cmd) do {                                                  \
                          int st25_retry = ST25_RETRY_NB;                     \
                          int st25_status = NFCTAG_ERROR;    \
                          while(st25_status != NFCTAG_OK)                     \
                          {                                                   \
                            st25_status = cmd;                                \
                            if(st25_status != NFCTAG_OK)                      \
                              HAL_Delay(ST25_RETRY_DELAY);                    \
                            if(st25_retry-- <= 0)                             \
                            {                                                 \
                              st25_retry = ST25_RETRY_NB;                     \
                            }                                                 \
                          }                                                   \
                      } while(0)



void ST25FTM_DeviceInit(void)
{
  ST25DV_EN_STATUS MB_mode = ST25DV_DISABLE;
  const ST25DV_PASSWD PassWord = {.MsbPasswd = 0x0, .LsbPasswd = 0x0};

  BSP_NFCTAG_PresentI2CPassword(0,PassWord );

  /* If not activated, activate Mailbox */
  BSP_NFCTAG_ReadMBMode(0,&MB_mode);
  if( MB_mode == ST25DV_DISABLE )
  {
    BSP_NFCTAG_WriteMBMode(0,ST25DV_ENABLE );
  }
  else
  {
    /* if already activated Clear MB content and flag */
    BSP_NFCTAG_ResetMBEN_Dyn(0);
    BSP_NFCTAG_SetMBEN_Dyn(0);
  }
  
  /* Disable MB watchdog feature */
  BSP_NFCTAG_WriteMBWDG(0,0);

  InitITGPOMode( ST25DV_GPO_RFPUTMSG_MASK | ST25DV_GPO_RFGETMSG_MASK | ST25DV_GPO_FIELDCHANGE_MASK | ST25DV_GPO_ENABLE_MASK );

}

/**
  * @brief  Writes message in Mailbox.
  * @param  pData Pointer to the data to write.
  * @param  NbBytes Number of bytes to write.
  * @return NFCTAG_StatusTypeDef status.
  */
ST25FTM_MessageStatus_t ST25FTM_WriteMessage(uint8_t* msg, uint32_t msg_len)
{
  int ret = NFCTAG_OK;
  ST25DV_MB_CTRL_DYN_STATUS data = {0};
  
  /* Check if Mailbox is available */
  ret = BSP_NFCTAG_ReadMBCtrl_Dyn(0, &data );
  if( ret != NFCTAG_OK )
  {
    return ST25FTM_MSG_ERROR;
  }
  
  /* If available, write data */
  if( (data.HostPutMsg == 0) && (data.RfPutMsg == 0) )
  {
    ret = BSP_NFCTAG_WriteMailboxData(0, msg, msg_len );
  } 
  else 
  {
    return ST25FTM_MSG_BUSY;
  }
  
  if(ret == NFCTAG_OK)
  {
    mailboxStatus = ST25FTM_MESSAGE_ME;
    return ST25FTM_MSG_OK;
  } else {
    return ST25FTM_MSG_ERROR;
  }
}

/**
  * @brief  Reads entire Mailbox Message from the tag.
  * @param  pData Pointer to the read data to store.
  * @param  pLength Number of bytes to read.
  * @return NFCTAG_StatusTypeDef status.
  */
ST25FTM_MessageStatus_t ST25FTM_ReadMessage(uint8_t *msg, uint32_t* msg_len)
{
  int ret = NFCTAG_OK;
  uint16_t mblength = 0;
  
  /* Read length of message */
  ret = BSP_NFCTAG_ReadMBLength_Dyn(0,  (uint8_t *)&mblength );
  if( ret != NFCTAG_OK )
  {
    return ST25FTM_MSG_ERROR;
  }
  *msg_len = mblength + 1;
  
  /* Read all data in Mailbox */
  ret = BSP_NFCTAG_ReadMailboxData(0, msg, 0, *msg_len );
  if(ret == NFCTAG_OK)
  {
    mailboxStatus = ST25FTM_MESSAGE_EMPTY;
    /* Trick to automatically detect the max frame length of the reader
       To have this auto detection working, the reader must send a long command
       before receiveing a long response.
    */
    ST25FTM_Ctrl_Byte_t ctrl;
    ctrl.byte = msg[0];
    if((!ST25FTM_CTRL_HAS_PKT_LEN(ctrl)) && !(msg[0] & ST25FTM_STATUS_BYTE))
    {
      ST25FTM_SetRxFrameMaxLength(*msg_len);
    }
    return ST25FTM_MSG_OK;
  }
  return ST25FTM_MSG_ERROR;
}


ST25FTM_MessageOwner_t ST25FTM_GetMessageOwner(void)
{
  /* Check if GPO IT has raised */
  ManageGPO();
  return mailboxStatus;
}

void ST25FTM_UpdateFieldStatus(void)
{
  ManageGPO();
  // first case: Both field transition occured, need to get the RF state from the register
  // second case: no RF transition, but as RF is supposed to be OFF, it doesn't harm to check the register
  if( ((FieldOffEvt == 1) && (FieldOnEvt == 1)) ||
      (((FieldOffEvt == 0) && (FieldOnEvt == 0)) && (gFtmState.rfField == ST25FTM_FIELD_OFF)))
  {
    // can't decide, need to read the register to get actual state 
    ST25DV_FIELD_STATUS field;
    int32_t status = BSP_NFCTAG_GetRFField_Dyn(0,&field);
    if((field == ST25DV_FIELD_ON) || (status == NFCTAG_NACK))
    {
      if(FieldOnEvt || FieldOffEvt)
        ST25FTM_LOG("FtmInfo Field Off->On\r\n");
      gFtmState.rfField = ST25FTM_FIELD_ON;
    } else {
      gFtmState.rfField = ST25FTM_FIELD_OFF;
      if(FieldOnEvt || FieldOffEvt)
        ST25FTM_LOG("FtmInfo Field On->Off\r\n");
    }
    FieldOnEvt = 0;
    FieldOffEvt = 0;

  }
  // Field transition to OFF
  if( (FieldOffEvt == 1) &&  (FieldOnEvt == 0) )
  {
    FieldOffEvt = 0;
    gFtmState.rfField = ST25FTM_FIELD_OFF;
    ST25FTM_LOG("FtmInfo Field Off\r\n");
    return;
  }
  // Field transition to ON
  if( (FieldOffEvt == 0) && (FieldOnEvt == 1) )
  {
    FieldOnEvt = 0;
    gFtmState.rfField = ST25FTM_FIELD_ON;
    ST25FTM_LOG("FtmInfo Field On\r\n");
  }
}


/**
  * @brief  Enable & initialize the GPO interrupt.
  * @param  ITConfig Value of the interrupt register to configure.
  * @return int32_t status.
  */
void MX_GPIO_Init(const uint16_t ITConfig)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = ITConfig;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}
static int32_t InitITGPOMode(const uint16_t ITConfig)
{
  int32_t ret = NFCTAG_OK;
  ST25DV_UID uid;
  uint8_t ProductRef;
  
  /* Init GPO Activation Flag */
  GPO_Activated = 0;
  // set both on/off events to 1, to force field register read.
  FieldOffEvt = 1;
  FieldOnEvt = 1;

  /* set correct edge setting depending on GPO technology */
  ST25_RETRY(BSP_NFCTAG_ReadUID(BSP_NFCTAG_INSTANCE, &uid));
  ProductRef = (uid.MsbUid & 0xFF00) >> 8;


  if(ST25DV_AM_I_OPEN_DRAIN(ProductRef))
  {
    /* GPO Open drain */
	  MX_GPIO_Init(GPIO_MODE_IT_FALLING);
  } 
  else 
  {
    /* GPO CMOS */
	  MX_GPIO_Init(GPIO_MODE_IT_RISING);
  }
  
  /* Set GPO Configuration */
  ret = BSP_NFCTAG_ConfigIT(BSP_NFCTAG_INSTANCE, ITConfig);
  /* Enable interruption */
  HAL_NVIC_SetPriority(BSP_GPO_EXTI, 0, 1);
  HAL_NVIC_EnableIRQ(BSP_GPO_EXTI);
  
  return ret;
}

/**
  * @brief  Reads the GPO interrupt source.
  * @details This function reads the interrupt status register from the ST25DV to report which interrupt(s) occured.
  * @param gpo Pointer on IT_GPO_STATUS structure, to return the status of the GPO irq.
  * @return None.
  */
static void ManageGPO( void )
{
  uint8_t itstatus;
 
  if(GPO_Activated == 1)
  {
    GPO_Activated = 0;

    ST25_RETRY(BSP_NFCTAG_ReadITSTStatus_Dyn(BSP_NFCTAG_INSTANCE, &itstatus));
      
    if((itstatus & ST25DV_ITSTS_DYN_FIELDFALLING_MASK) == ST25DV_ITSTS_DYN_FIELDFALLING_MASK)
    {
      FieldOffEvt = 1;
    }
    
    if((itstatus & ST25DV_ITSTS_DYN_FIELDRISING_MASK) == ST25DV_ITSTS_DYN_FIELDRISING_MASK)
    {
      FieldOnEvt = 1;
    }
    
    if((itstatus & ST25DV_ITSTS_DYN_RFPUTMSG_MASK) == ST25DV_ITSTS_DYN_RFPUTMSG_MASK)
    {

      mailboxStatus = ST25FTM_MESSAGE_PEER;
    }
    
    if((itstatus & ST25DV_ITSTS_DYN_RFGETMSG_MASK) == ST25DV_ITSTS_DYN_RFGETMSG_MASK)
    {
      mailboxStatus = ST25FTM_MESSAGE_EMPTY;
    }
  }
}

/* CRC services */
void ST25FTM_CRC_Initialize(void)
{
  __HAL_RCC_CRC_CLK_ENABLE();
  hcrc.Instance = CRC;
  __HAL_CRC_RESET_HANDLE_STATE(&hcrc);

  hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
  hcrc.Init.GeneratingPolynomial    = 0x04C11DB7;
  hcrc.Init.CRCLength               = CRC_POLYLENGTH_32B;
  hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS;
  /**/
  (void)HAL_CRC_Init( &hcrc );
}

ST25FTM_Crc_t ST25FTM_GetCrc(uint8_t *data, uint32_t length, ST25FTM_crc_control_t crc_control)
{
  uint32_t crc;
  static uint8_t last_word[4];
  static uint8_t last_extra_bytes = 0;

  ST25FTM_LOG("GetCrc %x %d\r\n",data,length);
  if((crc_control == ST25FTM_CRC_START) || (crc_control == ST25FTM_CRC_ONESHOT))
  {
	/* Compute number of words */
	uint32_t nbWords = length / 4U;
    /* The uint32_t* cast is ok as buffer length is managed above */
    crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)data,nbWords);
	 /* Save remaining extra bytes */
    last_extra_bytes = (uint8_t)(length%4U);
    memcpy(last_word,&data[nbWords*4U],last_extra_bytes);
  } else {
	uint32_t nbWords;
    /* In accumulate mode max length is 256 bytes (mailbox length)
	   + last 3 extra bytes from a previous call */
	uint8_t temp_buff[260];
	/* starts with extra bytes from previous call, if any */
	memcpy(temp_buff, last_word, last_extra_bytes);
	/* Append the provided data */
	memcpy(&temp_buff[last_extra_bytes],data,length);
	/* Compute full length & number of words */
	length += last_extra_bytes;
	nbWords = length / 4U;
	crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)temp_buff,nbWords);
	/* Save remaining extra bytes */
	last_extra_bytes = (uint8_t)(length%4U);
	memcpy(last_word,&temp_buff[nbWords*4U],last_extra_bytes);
  }

  if((crc_control == ST25FTM_CRC_END) || (crc_control == ST25FTM_CRC_ONESHOT))
  {
	if(last_extra_bytes > 0)
	{
	   /* Complete the CRC computation with the remaining bytes padded with 0 */
       memset(&last_word[last_extra_bytes],0,sizeof(last_word) - last_extra_bytes);
	   crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)last_word,1);
	   last_extra_bytes = 0;
	}
  }

  return crc;

}
