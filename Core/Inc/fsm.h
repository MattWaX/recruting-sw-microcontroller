#include "cli.h"
#include "main.h"
#include "stdio.h"

#define SERIAL_MSG_DIM 8

#define HANDLE_HAL_ERROR(status)                                               \
        if (status == HAL_ERROR) {                                             \
                *state = Error;                                                \
                return;                                                        \
        }

#define ANA_LOG_LEN 150

enum FSM_State { Init, Wait_Request, Listening, Pause, Warning, Error };

void FSM(enum FSM_State *state, UART_HandleTypeDef *huart,
         enum CLI_STATE *cli_state, enum CLI_CMD *cli_cmd,
         ADC_HandleTypeDef *hadc, uint32_t *ana_log_arr);

void init(enum FSM_State *state, enum CLI_STATE *cli_state,
          enum CLI_CMD *cli_cmd, ADC_HandleTypeDef *hadc);
void wait_request(enum FSM_State *state, enum CLI_STATE *cli_state);
void listening(enum FSM_State *state, UART_HandleTypeDef *huart,
               enum CLI_STATE *cli_state, enum CLI_CMD *cli_cmd,
               ADC_HandleTypeDef *hadc, uint32_t *ana_log_arr);
void pause(enum FSM_State *state, enum CLI_STATE *cli_state);
void warning(enum FSM_State *state, UART_HandleTypeDef *huart);
void error(enum FSM_State *state, UART_HandleTypeDef *huart);
