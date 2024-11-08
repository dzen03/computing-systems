/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#define RX_BUFFER_SIZE 64
volatile uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxHead = 0;
volatile uint8_t rxTail = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart6;

/* USART6 init function */
void MX_USART6_UART_Init(void)
{
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 38400;
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
    // Start the interrupt-based reception
    HAL_UART_Receive_IT(&huart6, &rxBuffer[rxHead], 1);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==USART6)
    {
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

        /* USART6 interrupt Init */
        HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if(uartHandle->Instance==USART6)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART6_CLK_DISABLE();

        /**USART6 GPIO Configuration
        PC6     ------> USART6_TX
        PC7     ------> USART6_RX
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

        /* USART6 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART6_IRQn);
    }
}

/* USER CODE BEGIN 1 */
void UART_SendChar(char c) {
    HAL_UART_Transmit(&huart6, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

void UART_SendString(const char* c) {
    HAL_UART_Transmit(&huart6, (uint8_t *)c, strlen(c), HAL_MAX_DELAY);
}

char UART_ReceiveChar(void) {
    uint8_t c;
    if (HAL_UART_Receive(&huart6, &c, 1, 0) == HAL_OK) {
        return c;
    }
    return 0;  // Return 0 if no data
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        uint8_t nextHead = (rxHead + 1) % RX_BUFFER_SIZE;
        if (nextHead != rxTail) {  // Check for buffer overflow
            rxHead = nextHead;
        } else {
            // Handle buffer overflow if necessary
        }
        // Restart the interrupt to receive the next byte
        HAL_UART_Receive_IT(&huart6, &rxBuffer[rxHead], 1);
    }
}

char UART_ReceiveChar_IT(void) {
    if (rxHead != rxTail) {
        uint8_t c = rxBuffer[rxTail];
        rxTail = (rxTail + 1) % RX_BUFFER_SIZE;
        return c;
    }
    return 0;  // Return 0 if no data
}

void USART6_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart6);
}
/* USER CODE END 1 */
