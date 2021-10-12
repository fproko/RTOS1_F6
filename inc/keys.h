/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef KEYS_H_
#define KEYS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1
#define EV_TECLA 1
#define EV_LED   2
/* types ================================================================= */
typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;

typedef struct
{
    gpioMap_t tecla;			//config
} t_key_config;

typedef struct
{
    keys_ButtonState_t state;   //variables

    TickType_t time_down;		//timestamp of the last High to Low transition of the key
    TickType_t time_up;		    //timestamp of the last Low to High transition of the key
    TickType_t time_diff;	    //variables
} t_key_data;

typedef struct
{
    TickType_t tiempo_medido;    //Tiempo de pulsación de la tecla en Ticks
    uint8_t index;              //Indice de TEC o LED
    uint8_t evento;
} tKey;

/* methods ================================================================= */
void keys_Init( void );
TickType_t get_diff( uint32_t index );
void clear_diff( uint32_t index );

#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
