#include "main.h"

#define SERIAL_MSG_DIM 8

#define HANDLE_HAL_ERROR(status)                                               \
        if (status == HAL_ERROR) {                                                \
                *state = Error;                                                \
                return;                                                        \
        }

enum FSM_State { Init, Wait_Request, Listening, Pause, Warning, Error };

void FSM(enum FSM_State *state, UART_HandleTypeDef *huart,
         ADC_HandleTypeDef *hadc);

void init(enum FSM_State *state, ADC_HandleTypeDef *hadc);
void wait_request(enum FSM_State *state);
void listening(enum FSM_State *state, UART_HandleTypeDef *huart,
               ADC_HandleTypeDef *hadc);
void pause(enum FSM_State *state);
void warning(enum FSM_State *state, UART_HandleTypeDef *huart);
void error(enum FSM_State *state, UART_HandleTypeDef *huart);
