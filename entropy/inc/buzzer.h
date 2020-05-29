/*
 * buzzer.h
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_
#include <stdio.h>


#define BUZZER_HS_TIMER          LEDC_TIMER_0
#define BUZZER_HS_MODE           LEDC_HIGH_SPEED_MODE
#define BUZZER_HS_CH0_GPIO       (27)
#define BUZZER_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define BUZZER_FREQUENCY		 (3100)
#define BUZZER_DUTY_CYCLE         (4000)

void buzzer_init(void);

#endif /* INC_BUZZER_H_ */
