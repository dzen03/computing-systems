#ifndef INC_IO_H_
#define INC_IO_H_

#include "usart.h"

char RecieveChar(void) {
	if (GetIRQ()) {
		return UART_ReceiveChar_IT();
	}
	return UART_ReceiveChar();
}

uint8_t SendChar(char c) {
	if (GetIRQ()) {
		return UART_SendChar_IT(c);
	}
	return UART_SendChar(c);
}

uint8_t SendString(const char* c) {
	if (GetIRQ()) {
		return UART_SendString_IT(c);
	}
	return UART_SendString(c);
}

void RecieveString(char* out, uint8_t len, void (*exec_seq)(void)) {
	uint8_t ind = 0;
	while (1) {
	  char c = RecieveChar();

	  if (c) {
		  if (c == 127) { // backspace
			  ind = ind - 1 >= 0 ? ind - 1: 0;
			  SendString("\b \b");
			  continue;
		  }
		  SendChar(c);

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
