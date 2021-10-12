/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
#include "queue.h"
#include "semphr.h"

/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
#define SEPARACION_MS 150
#define T_SEPARACION pdMS_TO_TICKS(SEPARACION_MS)
#define PRINT_RATE_MS 500
#define PRINT_RATE pdMS_TO_TICKS(PRINT_RATE_MS)
#define EV_TECLA 1
#define EV_LED   2
#define WELCOME_MSG  "Ejercicio F_6.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error al crear la cola.\r\n"
#define MSG_ERROR_MTX "Error al crear el mutex.\r\n"
#define LED_ERROR LEDR
#define N_QUEUE 	8
/*==================[definiciones de datos internos]=========================*/
const gpioMap_t leds_t[] = {LED1, LED2};
const gpioMap_t gpio_t[] = {GPIO7,GPIO5};
QueueHandle_t queue_tec_pulsada,queue_print;
#if 0
SemaphoreHandle_t mutex;
#endif

/* Estructura que indica que tecla fue presionada y por cuanto tiempo. */
typedef struct
{
    TickType_t tiempo_medido;    //Tiempo de pulsación de la tecla en Ticks
    uint8_t index;              //Indice de TEC o LED
    uint8_t evento;
} tPrint;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_print( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
    printf( WELCOME_MSG );

    BaseType_t res;
    uint32_t i;

    // Crear tarea en freeRTOS
    res = xTaskCreate(
              tarea_led,                     // Funcion de la tarea a ejecutar
              ( const char * )"tarea_led_a",   // Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
              0,                          // Parametros de tarea
              tskIDLE_PRIORITY+1,         // Prioridad de la tarea
              0                           // Puntero a la tarea creada en el sistema
          );

    // Gestion de errores
    configASSERT( res == pdPASS );

    // Creo tarea unica de impresion
    res = xTaskCreate(
              tarea_print,                     // Funcion de la tarea a ejecutar
              ( const char * )"tarea_print",   // Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
              0,                          // Parametros de tarea
              tskIDLE_PRIORITY+1,         // Prioridad de la tarea
              0                           // Puntero a la tarea creada en el sistema
          );

    // Gestion de errores
    configASSERT( res == pdPASS );

    /* inicializo driver de teclas */
    keys_Init();

#if 0
    // crear mutex!
    mutex = xSemaphoreCreateMutex( );

    // Gestion de errores de mutex
    configASSERT( mutex != NULL );
#endif

    // Crear cola
    queue_tec_pulsada = xQueueCreate(N_QUEUE, sizeof(tPrint)); //Cola de tecla pulsada.
    queue_print = xQueueCreate(N_QUEUE, sizeof(tPrint));       //Cola de estructuras a imprimir.

    // Gestion de errores de colas
    configASSERT( queue_tec_pulsada != NULL );
    configASSERT( queue_print != NULL );

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/
void gpio_init( void )
{
    uint32_t i;

    for( i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t index;
    tPrint datos;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while (TRUE)
    {
        index = 1;
        if (xQueueReceive(queue_tec_pulsada, &datos, 0) == pdTRUE) // Espero 0 ticks por un dato nuevo
        {
            index = datos.index; //Indice de TEC que se corresponde con un LED
        }
        gpioWrite(leds_t[index], ON);
        vTaskDelay(xPeriodicity / 2);
        gpioWrite(leds_t[index], OFF);

        /* Arma estructura y la envia a la cola de print */
        datos.tiempo_medido = 0;
        datos.index = index + 1;
        datos.evento = EV_LED;
        xQueueSend(queue_print, &datos, portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
    }
}

void tarea_print( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity =  PRINT_RATE;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // ---------- REPETIR POR SIEMPRE --------------------------

    tPrint datos; //Se define para poder guarda la estructura que se quita de la cola

    while (TRUE)
    {
        xQueueReceive(queue_print, &datos, portMAX_DELAY); // Espera dato para imprimir
        
        switch (datos.evento)
        {
        case EV_TECLA:
            gpioWrite(GPIO5, ON);
            printf("Se pulso tecla %d, durante %d ms\r\n", (datos.index)+1, datos.tiempo_medido * portTICK_RATE_MS);
            gpioWrite(GPIO5, OFF);
            break;

        case EV_LED:
            gpioWrite(GPIO7, ON);
            printf("Se encendió led %d\r\n", datos.index);
            gpioWrite(GPIO7, OFF);
            break;

        default:
            break;
        }
        
        vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
