#ifndef INC_LED_H_
#define INC_LED_H_

#include <string.h>
#include <stdlib.h>
#include "io.h"
#include "usart.h"

#define MAX_SEQUENCES 8
#define MAX_STATES 8

typedef struct {
    char color;          // abc - gyr
    uint8_t brightness;  // Яркость от 0 до 100 (%)
} LED_Config;


LED_Config settings[9];
LED_Config current_config;

enum Mode { WORKING_MODE, SETTING_MODE };
enum Mode current_mode = WORKING_MODE;

uint8_t setting_stage = 0;
uint8_t selected_key = 0;
char input_color;
uint8_t input_brightness;

void initialize_settings() {
    settings[0] = (LED_Config){'a', 10};
    settings[1] = (LED_Config){'a', 40};
    settings[2] = (LED_Config){'a', 100};
    settings[3] = (LED_Config){'b', 10};
    settings[4] = (LED_Config){'b', 40};
    settings[5] = (LED_Config){'b', 100};
    settings[6] = (LED_Config){'c', 10};
    settings[7] = (LED_Config){'c', 40};
    settings[8] = (LED_Config){'c', 100};
}
void turn_off_leds(void) {
	htim4.Instance->CCR2 = htim4.Instance->CCR3 = htim4.Instance->CCR4 = 0;
}
void update_led(LED_Config config) {
	uint32_t pulse = config.brightness;
	turn_off_leds();
	switch (config.color) {
		case 'a':  // зеленый
			htim4.Instance->CCR2 = pulse;
			break;
		case 'b':  // желтый
			htim4.Instance->CCR3 = pulse;
			break;
		case 'c':  // красный
			htim4.Instance->CCR4 = pulse;
			break;
		default:
			break;
	}
}







#endif /* INC_LED_H_ */
