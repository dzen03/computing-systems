/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#define RX_BUFFER_SIZE 300
#define TX_BUFFER_SIZE 300

volatile uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxHead = 0;
volatile uint8_t rxTail = 0;

volatile uint8_t txBuffer[TX_BUFFER_SIZE];
volatile uint8_t txHead = 0;
volatile uint8_t txTail = 0;

volatile static uint8_t irq = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart6;

/* USART6 init function */

void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */
    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();

    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uint8_t UART_SendChar(char c) {
    return HAL_UART_Transmit(&huart6, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

uint8_t UART_SendString(const char* c) {
    return HAL_UART_Transmit(&huart6, (uint8_t *)c, strlen(c), HAL_MAX_DELAY);
}

uint8_t UART_SendChar_IT(char c) {
    uint8_t nextHead = (txHead + 1) % TX_BUFFER_SIZE;

    if (nextHead == txTail) {
        return 1;
    }

    txBuffer[txHead] = (uint8_t)c;
    txHead = nextHead;
    if (txHead == (txTail + 1) % TX_BUFFER_SIZE) {
        if (HAL_UART_Transmit_IT(&huart6, &txBuffer[txTail], 1) != HAL_OK) {
            return 1;
        }
    }

    return 0;
}

uint8_t UART_SendString_IT(const char* str) {
    while (*str) {
        if (UART_SendChar_IT(*str++) != 0) {
            return 1;
        }
    }
    return 0;
}

void UART_SendChar_Blocking(char c) {
    HAL_UART_Transmit(&huart6, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

void UART_SendString_Blocking(const char* str) {
    HAL_UART_Transmit(&huart6, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

char UART_ReceiveChar(void) {
    uint8_t c;
    if (HAL_UART_Receive(&huart6, &c, 1, 0) == HAL_OK) {
        return c;
    }
    return 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        uint8_t nextHead = (rxHead + 1) % RX_BUFFER_SIZE;
        if (nextHead != rxTail) {
            rxHead = nextHead;
        }
        HAL_UART_Receive_IT(&huart6, &rxBuffer[rxHead], 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        txTail = (txTail + 1) % TX_BUFFER_SIZE;
        if (txTail != txHead) {
            HAL_UART_Transmit_IT(&huart6, &txBuffer[txTail], 1);
        }
    }
}


char UART_ReceiveChar_IT(void) {
    if (rxHead != rxTail) {
        uint8_t c = rxBuffer[rxTail];
        rxTail = (rxTail + 1) % RX_BUFFER_SIZE;
        return c;
    }
    return 0;
}

void USART6_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart6);
}


void DisableIRQ(void) {
    HAL_UART_Abort(&huart6);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
    irq = 0;
}

void EnableIRQ(void) {
    HAL_NVIC_EnableIRQ(USART6_IRQn);
    rxHead = rxTail = 0;
    txHead = txTail = 0;
    HAL_UART_Receive_IT(&huart6, &rxBuffer[rxHead], 1);
    irq = 1;
}

uint8_t GetIRQ(void) {
    return irq;
}

/* USER CODE END 1 */
