/*
 * led.h
 *
 *  Created on: Oct 25, 2024
 *      Author: ilya
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#include <string.h>
#include <stdlib.h>
#include "io.h"
#include "usart.h"

#define MAX_SEQUENCES 8
#define MAX_STATES 8

typedef struct {
    char states[MAX_STATES];
    uint8_t stateCount;
    uint32_t period;
    uint8_t state;
} Sequence;

Sequence sequences[MAX_SEQUENCES] = {
		{.states={'r', 'y', 'g', 'n'}, .stateCount=4, .period=100},
		{.states={'r', 'n'}, .stateCount=2, .period=500},
		{.states={'g', 'n'}, .stateCount=2, .period=500},
		{.states={'y', 'n'}, .stateCount=2, .period=500}};
uint8_t activeSequence = 0;
static uint32_t lastTick = 0;
static uint32_t buttonStart = 0;

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


void ExecuteSequence(void) {
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 0 && buttonStart == 0) {
		buttonStart = HAL_GetTick();
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 1 && buttonStart != 0) {
		if (HAL_GetTick() - buttonStart > 50) {
			activeSequence = (activeSequence + 1) % MAX_SEQUENCES;
		}
		buttonStart = 0;
	}

    Sequence *seq = &sequences[activeSequence];

    if (seq->stateCount > 0) {
        uint32_t currentTick = HAL_GetTick();
        if ((currentTick - lastTick) >= seq->period) {
            char state = seq->states[seq->state];
            ControlLED(state);
			lastTick = currentTick;
			seq->state = (seq->state + 1) % seq->stateCount;
        }
    }
    else {
    	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
    }
}

void ProcessCommand(const char *command) {
    if (strncmp(command, "new ", 4) == 0) {
        uint8_t seqNum = activeSequence >= 4 ? activeSequence : 4 ;
        size_t len = strlen(command + 4);
        if (len >= 2 && len <= 8) {
            Sequence *seq = &sequences[seqNum];
            memcpy(seq->states, command + 4, len);
            seq->stateCount = len;

            char buf[10];

            UART_SendString("Enter delay (in ms): ");

            RecieveString(buf, 10, &ExecuteSequence);


            seq->period = atoi(buf);
            UART_SendString("\nOK ");
            UART_SendChar('0' + seqNum + 1);
            UART_SendChar('\n');
            return;
        }

    } else if (strncmp(command, "set interrupts on", 17) == 0) {
    	EnableIRQ();
        UART_SendString("\nOK\n");
        return;
    } else if (strncmp(command, "set interrupts off", 18) == 0) {
    	DisableIRQ();
        UART_SendString("\nOK\n");
        return;
    } else if (strncmp(command, "set ", 4) == 0) {
        uint8_t num = atoi(command + 4);
        if (num >= 1 && num <= MAX_SEQUENCES) {
            activeSequence = num - 1;
            UART_SendString("\nOK\n");
            return;
        }
    }

    UART_SendString("\nERR\n");
}


#endif /* INC_LED_H_ */
