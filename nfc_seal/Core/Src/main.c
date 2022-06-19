/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "seal_embedded.h"
#include "ili9341_cube.h"
#include "st25ftm_common.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
#define FTM_RESP_CMD (0x80)

typedef enum {
	FTM_VALID = ((uint8_t) 0x81), /*!> Command has been successfully processed */
	FTM_ERROR = ((uint8_t) 0x82), /*!> Failure when processing the command */
	FTM_INTERNAL_ERR = ((uint8_t) 0x83), /*!> Unused value */
	FTM_UNKNOWN = ((uint8_t) 0x84), /*!> Unknown command */
	FTM_NOT_ALLOWED = ((uint8_t) 0x85) /*!> The command is currently not allowed */
} ftm_status_t;

typedef enum {
	FTM_READ_ID = 0x0, /*!> Get FW and board version */
	FTM_SEND_PICTURE = 0x1, /*!> Send a picture to the FW */
	FTM_READ_PICTURE = 0x2, /*!> Read a picture from the FW */
	FTM_STOPWATCH = 0x3, /*!> Send timestamp */
	FTM_FW_UPGRADE = 0x4, /*!> Send a new FW */
	FTM_SEND_DATA = 0x5, /*!> Send data to the FW */
	FTM_READ_DATA = 0x6, /*!> Read data from the FW */
	FTM_SEND_PASSWORD = 0x7, /*!> Send password, required before the FW upgrade command */
	FTM_READ_PICTURE_NO_ERROR_RECOVERY = 0x08, /*!> Read data from the FW, not using Error recovery */
	FTM_READ_DATA_NO_ERROR_RECOVERY = 0x09, /*!> Read a picture from the FW, not using Error recovery */
	FTM_ECHO = 0x0F, /*!> Send and read data to/from teh FW, respomse data are the same than command. Error recovery is decided by 1st command byte */
	FTM_NO_COMMAND = 0xFF, /*!> Default value when no command is on-going */
} ftm_cmd_t;

static ftm_cmd_t current_command = FTM_NO_COMMAND;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

static uint8_t cmdBuffer[8192];
static uint32_t cmdLength = 8192;

static void FTM_ReadData_Callback(uint8_t *dst, uint8_t *src, uint32_t len);
static void FTM_ReadData_Callback2(uint8_t *dst, uint8_t *src, uint32_t len);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len) {
	int DataIdx;
	HAL_UART_Transmit(&huart1, ptr, len, HAL_MAX_DELAY);
	return len;
}
SE_PARMS *par;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */

uint8_t part1 = 0;
int cnt2 = 0;

int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_I2C1_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */

	BSP_NFCTAG_Init(0U);

	printf("\r============\r\n");

	if (BSP_NFCTAG_IsDeviceReady(0U, 2) != -1) {
		printf("\rNFC OK\r\n");

	} else {
		printf("\rNFC not ok\n");
	}

	ST25FTM_Init();

	ST25FTM_SetRxFrameMaxLength(256);
	ST25FTM_SetTxFrameMaxLength(256);
	cmdLength = 1000;
	current_command = FTM_NO_COMMAND;
	ST25FTM_ReceiveCommand(cmdBuffer, &cmdLength, NULL);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	float g[4096];
	float input[]    = {24.94, 24.75, 24.56, 25.38, 121, 34, 53, 40,  0.08,  0.19,  0.06,  0.06,  390, 0.769231, 0,  0};

	for (int i = 0; i < 4096; i++)
		g[i] = input[i%16];

	double scale = pow(2, 25);

	par = se_setup(8192, 6, scale, SE_SYM_ENCR);

	while (1)

	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		ST25FTM_Runner();

		if (ST25FTM_IsNewFrame()) {
			current_command = (ftm_cmd_t) cmdBuffer[0];
		}

		if (ST25FTM_IsReceptionComplete()) {

			if (current_command == FTM_READ_ID) {

				uint8_t response_p[6];
				response_p[2] = 0;
				response_p[3] = 1;
				response_p[4] = 2;
				response_p[5] = 3;
				response_p[0] = (FTM_RESP_CMD | current_command);
				response_p[1] = (FTM_VALID);

				ST25FTM_SendCommand(response_p, 6, 0, NULL);

			} else if (current_command == FTM_READ_DATA) {

				int data_transfer_length = 32768;
				uint32_t rspLen = data_transfer_length + 2;

				ST25FTM_Send_Ack_t ack =(current_command == FTM_READ_DATA) ?ST25FTM_SEND_WITH_ACK :ST25FTM_SEND_WITHOUT_ACK;

               if(cnt2%2==0){
            	   part1++;
            	   se_encrypt(NULL, &g, 4096 * 8, 1, par);
            	   ST25FTM_SendCommand(0, rspLen, ack, &FTM_ReadData_Callback);
               }
               else{
            	   ST25FTM_SendCommand(0, rspLen, ack, &FTM_ReadData_Callback2);
               }
				cnt2++;
				printf("\r\npart %d of 12 \r\n",cnt2);

			}
		}

		if (ST25FTM_IsTransmissionComplete() || ST25FTM_IsIdle()) {

			cmdLength = 4;
			current_command = FTM_NO_COMMAND;
			ST25FTM_ReceiveCommand(cmdBuffer, &cmdLength, NULL);

		}

		if (ST25FTM_CheckError()) {
			printf("error\n");
		}
		/* USER CODE END 3 */
	}
}

static void FTM_ReadData_Callback(uint8_t *dst, uint8_t *src, uint32_t len) {

	if (src == 0)
	{

		dst[0] = (FTM_RESP_CMD) | (current_command);
		dst[1] = (FTM_VALID);
		uint8_t *t = ((void*)par->se_ptrs->c0_ptr) ;
		memcpy(&dst[2],t , len - 2);
	}
	else
	{
		uint8_t *t = ((void*)par->se_ptrs->c0_ptr) + (int)src;
		memcpy(dst,t-2 , len);
	}
}

static void FTM_ReadData_Callback2(uint8_t *dst, uint8_t *src, uint32_t len) {

	if (src == 0)
	{

		dst[0] = (FTM_RESP_CMD) | (current_command);
		dst[1] = (FTM_VALID);
		uint8_t *t = ((void*)par->se_ptrs->c1_ptr) ;
		memcpy(&dst[2],t , len - 2);
	}
	else
	{
		uint8_t *t = ((void*)par->se_ptrs->c1_ptr) + (int)src;
		memcpy(dst,t-2 , len);
	}
}


/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00401650;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin : PG6 */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

