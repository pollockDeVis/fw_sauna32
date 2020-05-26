/*
 * ble_manager.h
 *
 *  Created on: May 27, 2020
 *      Author: Pollock
 */

#ifndef INC_BLE_MANAGER_H_
#define INC_BLE_MANAGER_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,

    HRS_IDX_NB,
};

void ble_manager_init(void);


#endif /* INC_BLE_MANAGER_H_ */
