/*
 * custom_gpio.h
 *
 *  Created on: Oct 11, 2024
 *      Author: ilya
 */

#ifndef INC_CUSTOM_GPIO_H_
#define INC_CUSTOM_GPIO_H_

#include "stm32f4xx_hal.h"

void write_pin(GPIO_TypeDef* gpio, uint16_t pin, GPIO_PinState state) {
  if(state == GPIO_PIN_RESET)
  {
    gpio->BSRR = (uint32_t)(pin) << 16U;
  }
  else
  {
    gpio->BSRR = (pin);
  }
}

GPIO_PinState get_pin(GPIO_TypeDef* gpio, uint16_t pin) {
  return (((gpio->IDR) & (pin)) == GPIO_PIN_RESET ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void toggle_pin(GPIO_TypeDef* gpio, uint16_t pin) {
  uint32_t odr = gpio->ODR;
  gpio->BSRR = ((odr & pin) << 16U) | (~odr & pin);
}

#endif /* INC_CUSTOM_GPIO_H_ */
