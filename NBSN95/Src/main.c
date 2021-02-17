/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "usart.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "at.h"
#include "lowpower.h"
#include "nbInit.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define NBIOT
#define lowpower_enter
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t rxbuf = 0;
static uint8_t rxlen = 0;
static uint8_t rxDATA[300]={0};
static uint8_t uart2_recieve_flag = 0;

static uint8_t rxbuf_lp = 0;

static uint8_t pwd_time_count = 0;			//Password time count times

//static uint8_t iwdg_flag = 0;					//Feed the dog flag

static uint8_t task_num = _AT_IDLE;			//NB task directory

static uint8_t error_num = 0;				    //Error count

static uint8_t _tanchuang_flag = 0;			//NBIOT uncontrolled connection information
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_LPUART1_UART_Init();
  MX_IWDG_Init();
  MX_RTC_Init();
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */
	config_Get();
	reboot_information_print();
	product_information_print();
	led_on(1000);		
	HAL_Delay(3000);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)&rxbuf,RXSIZE);
	HAL_UART_Receive_DMA(&hlpuart1,(uint8_t*)&rxbuf_lp,RXSIZE);
	My_UARTEx_StopModeWakeUp(&huart2);		//Enable serial port wake up		
  /* USER CODE END 2 */
	
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#ifdef NBIOT
	task_num = _AT;
#endif
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
#ifdef NBIOT
		if(task_num != _AT_IDLE)
		{
			if(NBTASK(&task_num) == _AT_ERROR)
			{
				HAL_Delay(1000);
				error_num++;		
			}
			else
			{
				HAL_Delay(1000);
				error_num = 0;
			}
	  }
		
		if(error_num > 3)
		{
			task_num = _AT_NRB;
			error_num = 0;
			nb.net_flag = no_status;
		}
#endif
		if(sys.pwd_flag == 0 && uart2_recieve_flag)
		{
			uint32_t p[3]={0};
			rtrim((char*)rxDATA);
			p[0] = rxDATA[3]<<24 |  rxDATA[2]<<16 | rxDATA[1]<<8 | rxDATA[0];
			p[1] = rxDATA[7]<<24 |  rxDATA[6]<<16 | rxDATA[5]<<8 | rxDATA[4];
			if(strcmp((char*)p,(char*)sys.pwd) == 0 && strlen((char*)rxDATA) < 9)
			{
				user_main_printf("Password Correct");
				sys.pwd_flag = 1;
			}
			else
			{
				user_main_printf("Password Incorrect");
			}
		}
		else if(sys.pwd_flag == 1 && uart2_recieve_flag)		
		{
			ATEerror_t AT_State = ATInsPro((char*)rxDATA);
			if(AT_State == AT_OK)
				printf("%s\r\n",ATError_description[AT_OK]);
			else if(AT_State == AT_PARAM_ERROR)
				printf("%s",ATError_description[AT_PARAM_ERROR]);
			else if(AT_State == AT_BUSY_ERROR)
				printf("%s",ATError_description[AT_BUSY_ERROR]);
			else if(AT_State == AT_TEST_PARAM_OVERFLOW)
				printf("%s",ATError_description[AT_TEST_PARAM_OVERFLOW]);
			else if(AT_State == AT_RX_ERROR)
				printf("%s",ATError_description[AT_RX_ERROR]);
			else if(AT_State == AT_ERROR)
			{
				rxDATA[strlen((char*)rxDATA)] = '\n';
				HAL_UART_Transmit_DMA(&hlpuart1,(uint8_t*)rxDATA,strlen((char*)rxDATA));		
				HAL_Delay(500);
			}
		}
		if(uart2_recieve_flag)
		{
			rxlen = 0;
			uart2_recieve_flag = 0;
			HAL_UART_Receive_IT(&huart2,(uint8_t*)&rxbuf,RXSIZE);
			memset(rxDATA,0,sizeof(rxDATA));
		}

#ifdef lowpower_enter
		if(task_num == _AT_IDLE && uart2_recieve_flag==0)
		{
			LPM_EnterStopMode();
			SystemClock_Config();
		}
#endif
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &hlpuart1)
	{
		if(nb.recieve_flag == NB_BUSY)
		{
			nb.recieve_data[nb.rxlen++] = rxbuf_lp;
			if(rxbuf_lp == 'K' || rxbuf_lp == '>')
				nb.recieve_flag = NB_CMD_SUCC;
		}
		else if(task_num >= _AT_COAP_OPEN && task_num <= _AT_TCP_CLOSE)
		{
			memcpy(nb.recieve_data_server+strlen(nb.recieve_data_server),(char*)&rxbuf_lp,1);
			if(rxbuf_lp=='+')
				_tanchuang_flag = 1;
			if(_tanchuang_flag == 1)
			{
				if(rxbuf_lp=='\n')
				{
					_tanchuang_flag = 0;
					nb.recieve_flag = NB_RECIEVE;
				}
			}
		}
		else
			HAL_UART_Transmit(&huart2,(uint8_t*)&rxbuf_lp,RXSIZE,1);
		HAL_UART_Receive_DMA(&hlpuart1,(uint8_t*)&rxbuf_lp,RXSIZE);
	}
	else if(huart == &huart2)
	{
		rxDATA[rxlen++] = rxbuf;
		if(rxbuf == '\r' || rxbuf == '\n')
		{					
			uart2_recieve_flag = 1;		
		}
		HAL_UART_Receive_IT(&huart2,(uint8_t*)&rxbuf,RXSIZE);
	}
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(nb.net_flag == no_status)
		task_num = _AT;
	else if(nb.net_flag == success)
		task_num = _AT_CSQ;

	LPM_DisableStopMode();
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
	HAL_IWDG_Refresh(&hiwdg);
	
#ifdef NBIOT	
	if(sys.pwd_flag)
	{
		pwd_time_count++;
		if(pwd_time_count == 17)
		{
			sys.pwd_flag = 0;
			pwd_time_count = 0;
			HAL_UART_Transmit(&huart2,(uint8_t*)"Password timeout\r\n",20,20);
		}
	}
#endif	
	
	if(nb.net_flag == fail)
		task_num = _AT_CSQ;
	
	My_AlarmInit(18,1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_14)
	{
		sensor.exit_flag=1;
		if(sys.mod == model6)
			sensor.exit_count++;
		if(nb.net_flag == success && sys.mod != model6)
			task_num = _AT_CSQ;
		if(sys.mod != model6)
			LPM_DisableStopMode();
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart == (&hlpuart1))
	{
		user_main_error("hlpuart1 ERROR");
		uint32_t isrflags = READ_REG(huart->Instance->ISR);
		switch(huart->ErrorCode)
    {
				case HAL_UART_ERROR_NONE:
						user_main_error("HAL_UART_ERROR_NONE\r\n");
						break;
				case HAL_UART_ERROR_PE:
						user_main_error("HAL_UART_ERROR_PE\r\n");
						READ_REG(huart->Instance->RDR);//PE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);//���־
						break;
				case HAL_UART_ERROR_NE:
						user_main_error("HAL_UART_ERROR_NE\r\n");
						READ_REG(huart->Instance->RDR);//NE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);//���־
						break;
				case HAL_UART_ERROR_FE:
						user_main_error("HAL_UART_ERROR_FE\r\n");
						READ_REG(huart->Instance->RDR);//FE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);//���־
						break;
				case HAL_UART_ERROR_ORE:
						user_main_error("HAL_UART_ERROR_ORE\r\n");
						READ_REG(huart->Instance->RDR);//ORE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);//���־
						break;
				case HAL_UART_ERROR_DMA:
						user_main_error("HAL_UART_ERROR_DMA\r\n");
						break;
				default:
						user_main_error("other\r\n");
						break;
    }
		MX_LPUART1_UART_Init();
		HAL_UART_Receive_DMA(&hlpuart1,(uint8_t*)&rxbuf_lp,RXSIZE);
	}
	else	if(huart == (&huart2))
	{
		user_main_error("huart2 ERROR");
		uint32_t isrflags = READ_REG(huart->Instance->ISR);
		switch(huart->ErrorCode)
    {
				case HAL_UART_ERROR_NONE:
						user_main_error("HAL_UART_ERROR_NONE\r\n");
						break;
				case HAL_UART_ERROR_PE:
						user_main_error("HAL_UART_ERROR_PE\r\n");
						READ_REG(huart->Instance->RDR);//PE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);//���־
						break;
				case HAL_UART_ERROR_NE:
						user_main_error("HAL_UART_ERROR_NE\r\n");
						READ_REG(huart->Instance->RDR);//NE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);//���־
						break;
				case HAL_UART_ERROR_FE:
						user_main_error("HAL_UART_ERROR_FE\r\n");
						READ_REG(huart->Instance->RDR);//FE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);//���־
						break;
				case HAL_UART_ERROR_ORE:
						user_main_error("HAL_UART_ERROR_ORE\r\n");
						READ_REG(huart->Instance->RDR);//ORE���־���ڶ�����DR
						READ_REG(huart->Instance->TDR);
						__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);//���־
						break;
				case HAL_UART_ERROR_DMA:
						user_main_error("HAL_UART_ERROR_DMA\r\n");
						break;
				default:
						user_main_error("other\r\n");
						break;
    }
		MX_USART2_UART_Init();
		HAL_UART_Receive_IT(&huart2,(uint8_t*)&rxbuf,RXSIZE);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
