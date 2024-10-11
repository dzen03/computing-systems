/*
 * utills.h
 *
 *  Created on: Oct 11, 2024
 *      Author: ilya
 */

#ifndef INC_UTILLS_H_
#define INC_UTILLS_H_



#define STATE_COUNT 4

struct SavedState {
	  GPIO_PinState state[STATE_COUNT][2];
};

#endif /* INC_UTILLS_H_ */
