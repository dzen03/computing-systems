/*
 * led_strobe.h
 *
 *  Created on: Oct 11, 2024
 *      Author: ilya
 */

#ifndef INC_LED_STROBE_H_
#define INC_LED_STROBE_H_

#include "custom_gpio.h"
#include "utills.h"
#include "stdio.h"

static const int SLEEP_TIME = 50;

void busywait_with_button(int time, int* state, struct SavedState* saved_state, GPIO_PinState* prev_button)
{
  char breaking = 0;

  for (int i = 0; i < time / SLEEP_TIME; ++i) {
	GPIO_PinState button = get_pin(GPIOC, GPIO_PIN_15);
    if (button != *prev_button) {
      if (button == GPIO_PIN_SET) {
        *state = (*state + 1) % 4;
        printf("btn press; state: %d; saved: %d,%d\n", *state, saved_state->state[*state][0], saved_state->state[*state][1]);
        write_pin(GPIOD, GPIO_PIN_13, saved_state->state[*state][0]);
        write_pin(GPIOD, GPIO_PIN_13, saved_state->state[*state][1]);
        breaking = 1;
      }
      *prev_button = button;
    }
    HAL_Delay(SLEEP_TIME);

    if (breaking) {
    	break;
    }

  }
}

#define case_n(x, time_13, time_15, state, saved_state, prev_button)               \
case x:                                           \
  busywait_with_button(time_13, &state, &saved_state, &prev_button);                  \
  toggle_pin(GPIOD, GPIO_PIN_13);                 \
  saved_state.state[state][0] = !saved_state.state[state][0]; \
  busywait_with_button(time_15, &state, &saved_state, &prev_button);                  \
  toggle_pin(GPIOD, GPIO_PIN_15);                 \
  saved_state.state[state][1] = !saved_state.state[state][1]; \
  break;

#endif /* INC_LED_STROBE_H_ */
