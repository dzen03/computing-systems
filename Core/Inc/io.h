#ifndef INC_IO_H_
#define INC_IO_H_

#include "usart.h"

volatile static uint8_t irq = 0;

void DisableIRQ(void) {
	HAL_UART_Abort(&huart6);
	HAL_NVIC_DisableIRQ(USART6_IRQn);
	irq = 0;
}
void EnableIRQ(void) {
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	irq = 1;
}

char RecieveChar(void) {
	if (irq) {
		return UART_ReceiveChar_IT();
	}
	return UART_ReceiveChar();
}

void RecieveString(char* out, uint8_t len, void (*exec_seq)(void)) {
	uint8_t ind = 0;
	while (1) {
	  char c = RecieveChar();

	  if (c) {
		  if (c == 127) { // backspace
			  ind = ind - 1 >= 0 ? ind - 1: 0;
			  UART_SendString("\b \b");
			  continue;
		  }
		  UART_SendChar(c);

		  if (c == '\n' || c == '\r') {
			  out[ind] = '\0';
			  return;
		  } else {
			  out[ind++] = c;
			  if (ind >= len) {
				  ind = 0;
			  }
		  }
	  }

	  (*exec_seq)();
  }
}

#endif /* INC_IO_H_ */
