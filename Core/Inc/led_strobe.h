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

void busywait_with_button(int time, int* state, struct SavedState* saved_state)
{
  for (int i = 0; i < time / SLEEP_TIME; ++i) {
    if (get_pin(GPIOC, GPIO_PIN_15) == 0) {
      while(get_pin(GPIOC, GPIO_PIN_15) == 0) {
        HAL_Delay(SLEEP_TIME);
      }
      *state = (*state + 1) % 4;
      printf("btn press; state: %d; saved: %d,%d\n", *state, saved_state->state[*state][0], saved_state->state[*state][1]);
      write_pin(GPIOD, GPIO_PIN_13, 	saved_state->state[*state][0]);
      write_pin(GPIOD, GPIO_PIN_13, saved_state->state[*state][1]);
      break;
    }
    else {
      HAL_Delay(SLEEP_TIME);
    }
  }
}

#define case_n(x, time_13, time_15, state, saved_state)               \
case x:                                           \
  busywait_with_button(time_13, &state, &saved_state);                  \
  toggle_pin(GPIOD, GPIO_PIN_13);                 \
  saved_state.state[state][0] = !saved_state.state[state][0]; \
  busywait_with_button(time_15, &state, &saved_state);                  \
  toggle_pin(GPIOD, GPIO_PIN_15);                 \
  saved_state.state[state][1] = !saved_state.state[state][1]; \
  break;

#endif /* INC_LED_STROBE_H_ */
