#ifndef INC_IMP_H_
#define INC_IMP_H_

#include "i2c.h"
#include "usart.h"
#include "led.h"
#include "pca9538.h"


enum Mode { TEST_MODE, APPLICATION_MODE };
static enum Mode current_mode = TEST_MODE;

enum SettingsState { SETTING, LED, BRIGHTNESS};

struct Setting {
	int num;
	LED_Config conf;
};

struct Setting current_setting = {0};

#define DEBOUNCE_DELAY 400 // milliseconds
#define BUTTON_COUNT 12
#define MODE_SWITCH_GPIO_Port GPIOC
#define MODE_SWITCH_Pin GPIO_PIN_15
#define BUTTONS_I2C_ADDRESS 0xE2

// PCA9538 register addresses
#define PCA9538_INPUT_PORT       0x00
#define PCA9538_OUTPUT_PORT      0x01
#define PCA9538_POLARITY_INV     0x02
#define PCA9538_CONFIG_PORT      0x03

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7

static const uint8_t rows_map[4] = {
	ROW1,
	ROW2,
	ROW3,
	ROW4
};

// Button mapping matrix [row][col]
static const uint8_t button_map[4][3] = {
    {1,  2,  3},   // Row 0
    {4,  5,  6},   // Row 1
    {7,  8,  9},   // Row 2
    {10, 11, 12}   // Row 3
};

static uint32_t last_mode_switch_time = 0;
static uint8_t last_button_state = 0;
static uint32_t last_button_time = 0;

HAL_StatusTypeDef Set_Keyboard( void ) {
	HAL_StatusTypeDef ret;
	uint8_t buf;

	buf = 0;
	ret = PCA9538_Write_Register(BUTTONS_I2C_ADDRESS, POLARITY_INVERSION, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write polarity\r\n");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
		goto exit;
	}

	buf = 0;
	ret = PCA9538_Write_Register(BUTTONS_I2C_ADDRESS, PCA9538_OUTPUT_PORT, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write output\r\n");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}

exit:
	return ret;
}

uint8_t Check_Row(uint8_t Nrow) {
    uint8_t Nkey = 0x00;
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;
    uint8_t kbd_in;
    int cnt = 0;

    ret = Set_Keyboard();
    if (ret != HAL_OK) {
        HAL_UART_Transmit(&huart6, (uint8_t *)"Error write init\r\n", 17, 30);
        return 0x0F;
    }

    buf = Nrow;
    ret = PCA9538_Write_Register(BUTTONS_I2C_ADDRESS, PCA9538_CONFIG_PORT, &buf);
    if (ret != HAL_OK) {
        HAL_UART_Transmit(&huart6, (uint8_t *)"Error write config\r\n", 19, 30);
        return 0x0F;
    }

    ret = PCA9538_Read_Inputs(BUTTONS_I2C_ADDRESS, &buf);
    if (ret != HAL_OK) {
        HAL_UART_Transmit(&huart6, (uint8_t *)"Read error\r\n", 11, 30);
        return 0x0F;
    }

    kbd_in = buf & 0x70;
    Nkey = kbd_in;
    if (kbd_in != 0x70) {
        if (!(kbd_in & 0x10)) {
            Nkey = (Nrow == ROW1) ? 1 : (Nrow == ROW2) ? 4 : (Nrow == ROW3) ? 7 : 10;
            cnt++;
        }
        if (!(kbd_in & 0x20)) {
            Nkey = (Nrow == ROW1) ? 2 : (Nrow == ROW2) ? 5 : (Nrow == ROW3) ? 8 : 11;
            cnt++;
        }
        if (!(kbd_in & 0x40)) {
            Nkey = (Nrow == ROW1) ? 3 : (Nrow == ROW2) ? 6 : (Nrow == ROW3) ? 9 : 12;
            cnt++;
        }
    } else {
        Nkey = 13;
    }

    if (cnt > 1) {
        Nkey = 0xFF;
    }

    return Nkey;
}

void ToggleMode(void) {
    if (current_mode == APPLICATION_MODE) {
        current_mode = TEST_MODE;
        SendString("Режим: тестирование клавиатуры\r\n");
    } else {
        current_mode = APPLICATION_MODE;
        SendString("Режим: прикладной\r\n");
    }
}

uint8_t CheckModeSwitchButton(void) {
    // Implement reading the side panel button (GPIO with debounce)
    uint8_t state = HAL_GPIO_ReadPin(MODE_SWITCH_GPIO_Port, MODE_SWITCH_Pin);
    if (state == GPIO_PIN_RESET && (HAL_GetTick() - last_mode_switch_time) > DEBOUNCE_DELAY) {
        last_mode_switch_time = HAL_GetTick();
        return 1;
    }
    return 0;
}

uint8_t ReadButtons(void) {
    uint8_t button_code = 0;
    uint8_t config;
    uint8_t output;
    uint8_t input;

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t res = Check_Row(rows_map[row]);
        if (res != 0x0F && res != 0x0D) {
//        	return res;
            if (last_button_state != res || (HAL_GetTick() - last_button_time) > DEBOUNCE_DELAY) {
            	last_button_state = res;
            	last_button_time = HAL_GetTick();
                return res;
            }
        }
    }

    return 0;
}

enum SettingsState ss = SETTING;

void ProcessButtonInput(uint8_t button_code) {

	static char message[300];
    if (button_code != 0) {
    	if (current_working_mode == WORKING_MODE) {
			// Perform action based on button code
			if (button_code >= 1 && button_code <= 9) {
				uint8_t key = button_code - 1;
				current_config = settings[key];
				update_led(current_config);
				sprintf(message, "Включен светодиод '%c' с яркостью %d%%.\r\n",
						current_config.color, current_config.brightness);
				SendString(message);
			} else if (button_code == 10) {
				turn_off_leds();
				SendString("Выключены все светодиоды.\r\n");
			} else if (button_code == 11) {
				current_working_mode = SETTING_MODE;
				sprintf(message, "Режим настроек, введите номер настроек.\r\n",
						current_config.color, current_config.brightness);
				SendString(message);
			}
    	}
    	else {
//    		current_setting = (Setting){0};
    		if (ss == SETTING) {
    			if (button_code >= 1 && button_code <= 9) {
    				current_setting.num = button_code - 1;
    				sprintf(message, "%d. Теперь введите светодиод ('a'|'b'|'c'): \r\n", current_setting.num + 1);
    				SendString(message);
    				ss = LED;
    			}
    		}
    		else if (ss == LED) {
    			if (button_code >= 1 && button_code <= 3) {
    				current_setting.conf.color = button_code + 'a' - 1;
    				sprintf(message, "%c. Теперь введите яркость ('+'|'-')\r\n", current_setting.conf.color);
    				SendString(message);
    				current_setting.conf.brightness = 50;
    				ss = BRIGHTNESS;
				}
    		} else if (ss == BRIGHTNESS) {
    			if (button_code == 1) {
    				current_setting.conf.brightness -= 10;
    			} else if (button_code == 2) {
    				current_setting.conf.brightness += 10;
    			} else if (button_code == 3) {
    				settings[current_setting.num] = current_setting.conf;
    				sprintf(message, "Теперь пресет №%d: светодиод '%c' с яркостью %d%%.\r\n",
    						current_setting.num + 1, current_setting.conf.color, current_setting.conf.brightness);
				    SendString(message);
				    ss == SETTING;
				    current_working_mode = WORKING_MODE;
    			}
    			char message[100];
    			sprintf(message, "\tЯркость: %d%%\r\n", current_setting.conf.brightness);
    			SendString(message);
    		}
    	}
        // Handle other button codes if necessary
    }
}

#endif /* INC_IMP_H_ */
