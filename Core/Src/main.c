/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "usart.h"
#include "io.h"
#include "imp.h"

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
//extern void initialise_monitor_handles(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	initialise_monitor_handles();
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
  MX_USART6_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  initialize_settings();
  EnableIRQ();
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
  SendString("\r\nСтарт.\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  static char message[500] = {0};
  while (1) {
	  char c = RecieveChar();

	  if (c) {
		  if (c >= '1' && c <= '9') {
			  uint8_t key = c - '1';
			  current_config = settings[key];
			  update_led(current_config);
			  sprintf(message, "Включен светодиод '%c' с яркостью %d%%.\r\n", current_config.color, current_config.brightness);
			  SendString(message);
		  } else if (c == '0') {
			  turn_off_leds();
			  sprintf(message, "Выключены все светодиоды.\r\n");
			  SendString(message);
		  } else if (c == '\r') {

			  READ_NUM:
			  sprintf(message, "Режим настроек. Введите номер настройки для изменения [1 - 9]: ");
			  SendString(message);


			  for (c = RecieveChar(); c == 0; c = RecieveChar()) {}

			  if (c < '1' || c > '9') {
				  sprintf(message, "%c. Некорректный ввод.\r\n", c);
				  SendString(message);
				  goto READ_NUM;
			  }

			  const int settings_id = c - '1';

			  READ_LED:
			  sprintf(message, "%d.\r\nТеперь введите светодиод ('a'|'b'|'c'): ", settings_id + 1);
			  SendString(message);

			  for (c = RecieveChar(); c == 0; c = RecieveChar()) {}

			  if (c != 'a' && c != 'b' && c != 'c') {
				  sprintf(message, "%d. Некорректный ввод.\r\n", settings_id + 1);
				  SendString(message);
				  goto READ_LED;
			  }

			  const char led_id = c;
			  sprintf(message, "%c.\r\nТеперь введите яркость ('+'|'-')\r\n", led_id);
			  SendString(message);

			  int pulse = 50;
			  int br = 0;

			  while(!br) {
				  sprintf(message, "\tЯркость: %d%%\r\n", pulse);
				  SendString(message);
				  for (c = RecieveChar(); c == 0; c = RecieveChar()) {}



				  switch (c){
					  case '\r':
						  br = 1;
						  break;
					  case '+':
						  pulse += (pulse + 10 <= 100 ? 10 : 0);
						  break;
					  case '-':
						  pulse -= (pulse - 10 >= 0 ? 10 : 0);
						  break;
					  default:
						  sprintf(message, "\tНекорректный ввод: %c\r\n", c);
						  SendString(message);
				  }
			  }

			  settings[settings_id].color = led_id;
			  settings[settings_id].brightness = pulse;

			  sprintf(message, "Теперь пресет №%d: светодиод '%c' с яркостью %d%%.\r\n",
					  settings_id + 1, settings[settings_id].color, settings[settings_id].brightness);
			  SendString(message);
		  } else {
			  sprintf(message, "Некорректный ввод: '%c'.\r\n", c);
			  SendString(message);
		  }
	  }

      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 108;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
