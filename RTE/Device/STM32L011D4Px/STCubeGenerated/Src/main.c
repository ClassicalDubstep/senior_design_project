/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef hlpuart1;

WWDG_HandleTypeDef hwwdg;

/* USER CODE BEGIN PV */
const uint8_t volume_levels[15] = {0, 3, 10, 15, 24, 33, 45, 58, 75, 94, 117, 144, 176, 214, 0};  // logarithmic volume levels in 1.5dB steps
const uint16_t rheostat_address = 0x58, charger_address = 0x6B; // I2C slave addresses
uint8_t UART_Tx_data[2], UART_Rx_data[8], rheostat_data[8]; // UART and I2C data byte arrays
uint8_t current_volume = 0;
uint32_t time0 = 0, time1 = 0, time2 = 0, poll_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_WWDG_Init(void);

/* USER CODE BEGIN PFP */
static void power_state_handler(void);
static void UART_Command_Handler(void);
static void Bass_Volume(uint8_t volume);
uint8_t Button_State(GPIO_PinState button);
uint32_t Button_Duration(GPIO_PinState button);
uint8_t Bass_Adjust(GPIO_PinState button);
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
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_IWDG_Init();
  MX_LPUART1_UART_Init();
  MX_WWDG_Init();
  /* USER CODE BEGIN 2 */
	Bass_Volume(0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		

    /* USER CODE BEGIN 3 */
		UART_Command_Handler();
		if(HAL_GPIO_ReadPin(BASS_UP_GPIO_Port, BASS_UP_Pin) == GPIO_PIN_SET)
		{
			if((current_volume + Bass_Adjust(HAL_GPIO_ReadPin(BASS_UP_GPIO_Port, BASS_UP_Pin))) < 14)
				Bass_Volume(Bass_Adjust(HAL_GPIO_ReadPin(BASS_UP_GPIO_Port, BASS_UP_Pin)) + current_volume);
			else
				Bass_Volume(14);
		}
		else if(HAL_GPIO_ReadPin(BASS_DN_GPIO_Port, BASS_DN_Pin) == GPIO_PIN_SET)
		{
			if((current_volume - Bass_Adjust(HAL_GPIO_ReadPin(BASS_DN_GPIO_Port, BASS_DN_Pin))) > 0)
				Bass_Volume(Bass_Adjust(HAL_GPIO_ReadPin(BASS_DN_GPIO_Port, BASS_DN_Pin)) + current_volume);
			else
				Bass_Volume(0);
		}
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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

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
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
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
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000107;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /** I2C Fast mode Plus enable 
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 209700;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_7B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief WWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_WWDG_Init(void)
{

  /* USER CODE BEGIN WWDG_Init 0 */

  /* USER CODE END WWDG_Init 0 */

  /* USER CODE BEGIN WWDG_Init 1 */

  /* USER CODE END WWDG_Init 1 */
  hwwdg.Instance = WWDG;
  hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
  hwwdg.Init.Window = 64;
  hwwdg.Init.Counter = 64;
  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
  if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN WWDG_Init 2 */

  /* USER CODE END WWDG_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : INT_Pin */
  GPIO_InitStruct.Pin = INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AMP_EN_Pin */
  GPIO_InitStruct.Pin = AMP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AMP_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MFB_Pin */
  GPIO_InitStruct.Pin = MFB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MFB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BASS_UP_Pin BASS_DN_Pin */
  GPIO_InitStruct.Pin = BASS_UP_Pin|BASS_DN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */
/* switches MCU between power states: run, low power run, and standby */
static void power_state_handler(void)
{
	poll_counter++;
	if(poll_counter == 500000)
	{
		/* switch MCU to standby mode if BTM is in powered off state */
		if(UART_Rx_data[1] == 0x00)	
		{
			HAL_PWR_EnableWakeUpPin(MFB_Pin);
			HAL_PWR_EnterSTANDBYMode();
		}
		
		else
		{
			/* swtich MCU to low power run mode and shutdown amplifier if no audio streaming */
			if(UART_Rx_data[7] == 0x00)
			{
				HAL_PWREx_EnableLowPowerRunMode();
				HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_RESET);
				
			}
			/* swtich MCU to run mode and enable amplifier if there is audio streaming */
			else if(UART_Rx_data[7] == 0x01)
			{
				HAL_PWREx_DisableLowPowerRunMode();
				HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_SET);
			}
		}
	}
}

/* poll BTM status every 500ms */
static void UART_Command_Handler(void)
{
	if((HAL_GetTick()-time0) >= 500 || time0 == 0)
	{
		UART_Tx_data[0] = 0x0D;
	
		HAL_UART_Transmit(&hlpuart1, UART_Tx_data, 	2, 50);
		HAL_UART_Receive(&hlpuart1, UART_Rx_data, 8, 50);
		if(UART_Rx_data[0] == 0x1E)
		{
			power_state_handler();
		}
		time0 = HAL_GetTick(); //reset timer
	}
}

/* set bass volume using digital rheostat IC */
static void Bass_Volume(uint8_t volume)
{
	for(uint8_t i=0; i<=7; i++) //set rheostat data array
	{
		 if(i%2 == 0)
		 {
			 switch (i)
			 {
				 case 0: 
					 rheostat_data[i] = 0x00; //P0
					 break;
				 case 2: 
					 rheostat_data[i] = 0x10; //P1
					 break;
				 case 4: 
					 rheostat_data[i] = 0x20; //P2
					 break;
				 case 6: 
					 rheostat_data[i] = 0x30; //P3
					 break;
			 }
			 if(volume == 14) 
				 rheostat_data[i]++; //256
		 }
		 else 
			 rheostat_data[i] = volume_levels[volume]; //set resistor level
	}
	if(poll_counter == 500000)
	{
		HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_RESET);
		HAL_I2C_Master_Transmit(&hi2c1, rheostat_address, rheostat_data, 8, 50); //do I2C transmission to MCP4452
		HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_SET);
		poll_counter = 0;
	}
}
	
/* returns: 0 = released, 1 = rising edge, 2 = pressed, 3 = falling edge */
uint8_t Button_State(GPIO_PinState button)
{
	GPIO_PinState button_previous = button;
	if(button == button_previous)
	{
		if(button == GPIO_PIN_SET)
			return 0;
		else
			return 3;
	}
	else
	{
		if(button == GPIO_PIN_SET)
			return 1;
		else
			return 2;
	}
}

/* returns duration button was pressed */
uint32_t Button_Duration(GPIO_PinState button)
{
	switch (Button_State(button))
	{
		case 0:
			time1 = 0; 
			time2 = 0;
			break;
		case 1:
			time1 = HAL_GetTick();
			break;
		case 2:
			break;
		case 3:
			time2 = HAL_GetTick();
			break;
		default:
			break;
	}
	return (time2 - time1);
}

/* returns button repeat amount */
uint8_t Bass_Adjust(GPIO_PinState button)
{
	uint8_t button_repeat = 0;
	while(Button_State(button) == 1 || Button_State(button) == 2)
	{
		button_repeat = Button_Duration(button)/500; //button repeat accumulator ever 500ms while pressed
	}
	return button_repeat;
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