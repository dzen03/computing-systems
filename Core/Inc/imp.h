#ifndef INC_IMP_H_
#define INC_IMP_H_

#include "led.h"

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
uint8_t newPush = 4;
static uint32_t lastTick = 0;
static uint32_t buttonStart = 0;

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
        size_t len = strlen(command + 4);
        if (len >= 2 && len <= 8) {
            Sequence *seq = &sequences[newPush];
            memcpy(seq->states, command + 4, len);
            seq->stateCount = len;

            char buf[10];

            SendString("Enter delay (in ms): ");

            RecieveString(buf, 10, &ExecuteSequence);


            seq->period = atoi(buf);
            SendString("\nOK ");
            SendChar('0' + newPush + 1);
            SendChar('\n');

            newPush = (newPush + 1) % MAX_STATES;
            newPush = (newPush >= 4 ? newPush : 4);
            return;
        }

    } else if (strncmp(command, "set interrupts on", 17) == 0) {
    	EnableIRQ();
    	SendString("\nOK\n");
        return;
    } else if (strncmp(command, "set interrupts off", 18) == 0) {
    	DisableIRQ();
        SendString("\nOK\n");
        return;
    } else if (strncmp(command, "set ", 4) == 0) {
        uint8_t num = atoi(command + 4);
        if (num >= 1 && num <= MAX_SEQUENCES) {
            activeSequence = num - 1;
            SendString("\nOK\n");
            return;
        }
    }

    SendString("\nERR\n");
}

#endif /* INC_IMP_H_ */
