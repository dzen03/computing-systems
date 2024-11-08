#ifndef INC_LED_H_
#define INC_LED_H_

#include <string.h>
#include <stdlib.h>
#include "io.h"
#include "usart.h"

#define MAX_SEQUENCES 8
#define MAX_STATES 8



uint8_t ControlLED(char state) {
    switch (state) {
        case 'g':
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            return 1;
        case 'y':
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
            return 1;
        case 'r':
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
            return 1;
        case 'n':
        	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
        	return 1;
        default:
        	return 0;
    }
}





#endif /* INC_LED_H_ */
