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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ring_buffer.h"
#include "bh1750.h"
#include "helpers.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static char message[] = "Hello World!\r\n";
// UART transmit buffer descriptor
RingBuffer UART_RingBuffer_Tx;
// UART transmit buffer memory pool
char RingBufferData_Tx[1024];

// UART receive buffer descriptor
RingBuffer UART_RingBuffer_Rx;
// UART receive buffer memory pool
char RingBufferData_Rx[1024];

char Response_Rx[1024];
uint8_t uart_rx_buffer;

uint8_t readyToRead;
uint8_t readyToSend;

BH1750_t sensor1;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart2) {
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart2) {
	  char_append(uart_rx_buffer);
    HAL_UART_Receive_IT(&huart2, &uart_rx_buffer, 1);
  }
}
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
  RingBuffer_Init(&UART_RingBuffer_Tx, &RingBufferData_Tx, 1024);
  RingBuffer_Init(&UART_RingBuffer_Rx, &RingBufferData_Rx, 1024);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  BH1750_t sensor1 = 
  {
    .BH1750_i2c = &hi2c2,
    .BH1750_Mode = CONTINUOUS_H_RESOLUTION_MODE_2,
    .BH1750_MTreg = BH1750_DEFAULT_MTREG,
    .BH1750_ADDR = (0x23<<1)
  };

  BH1750_Init(&sensor1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  UART_WriteString(message);
  readyToRead = 0;
  readyToSend = 0;

  HAL_UART_Receive_IT(&huart2, &uart_rx_buffer, 1);

  float BH1750_lux;
	
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(readyToRead){
      UART_ReadResponse();
      readyToRead = 0;
    }

    if (readyToSend)
    {
      char toSend[32];
      if (readyToSend == 1) // read value from light sensor
      {
        if (BH1750_OK == BH1750_Read(&sensor1, &BH1750_lux))
        {
          uint16_t finalSize = sprintf(toSend, "%.2f\r\n", BH1750_lux);
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
        else {
          uint16_t finalSize = sprintf(toSend, "ERROR: sensor read\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
      }
      else if (readyToSend == 2) // increase sensitivity
      {

        if (BH1750_OK == BH1750_setMTreg(&sensor1, (sensor1.BH1750_MTreg + 10)))
        {
          uint16_t finalSize = sprintf(toSend, "Sensitivity increased.\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
        else {
          uint16_t finalSize = sprintf(toSend, "ERROR: unable to incSens\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
      }
      else if (readyToSend == 3) // decrease sensitivity
      {
        if (BH1750_OK == BH1750_setMTreg(&sensor1, (sensor1.BH1750_MTreg - 10)))
        {
          uint16_t finalSize = sprintf(toSend, "Sensitivity decreased.\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
        else {
          uint16_t finalSize = sprintf(toSend, "ERROR: unable to decSens\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
        }
      }
      else if (readyToSend == 4) // help
      {
        char toSend[130];
        uint16_t finalSize = sprintf(toSend, "Accessible commands:;- readValue - read value rom light sensor;- incSens - increase sensitivity;- decSens - decrease sensitivity\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
      }
      else if (readyToSend == 5) //unknown command
      {
        uint16_t finalSize = sprintf(toSend, "ERROR: command not found\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t *)toSend, finalSize, 100);
      }

      readyToSend = 0;
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_3;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_3;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
