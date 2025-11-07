#include "fsm.h"

void FSM(enum FSM_State *state, UART_HandleTypeDef *huart,
         enum CLI_STATE *cli_state, enum CLI_CMD *cli_cmd,
         ADC_HandleTypeDef *hadc, uint32_t *ana_log_arr) {
        switch (*state) {
        case Init:
                init(state, cli_state, cli_cmd, hadc);
                break;

        case Wait_Request:
                wait_request(state, cli_state);
                break;

        case Listening:
                listening(state, huart, cli_state, cli_cmd, hadc, ana_log_arr);
                break;

        case Pause:
                pause(state, cli_state);
                break;

        case Warning:
                warning(state, huart);
                break;

        case Error:
                error(state, huart);
                break;
        }
}

void init(enum FSM_State *state, enum CLI_STATE *cli_state,
          enum CLI_CMD *cli_cmd, ADC_HandleTypeDef *hadc) {
        HANDLE_HAL_ERROR(HAL_ADCEx_Calibration_Start(hadc));

        *cli_state = CLI_OFF;
        *cli_cmd = CLI_RAW;

        *state = Wait_Request;
}

void wait_request(enum FSM_State *state, enum CLI_STATE *cli_state) {
        // Led Pin Off
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        // cli on
        *cli_state = CLI_ON;

        // On button press transition to listening
        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
                *state = Listening;
}

void listening(enum FSM_State *state, UART_HandleTypeDef *huart,
               enum CLI_STATE *cli_state, enum CLI_CMD *cli_cmd,
               ADC_HandleTypeDef *hadc, uint32_t *ana_log_arr) {
        // Duty Cycle = 100% = ARR
        TIM1->CCR1 = TIM1->ARR;
        // Led Pin On
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        // cli off
        *cli_state = CLI_OFF;

        // On button press transition to pause
        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {
                *state = Pause;
                return;
        }

        uint32_t AD_val;
        uint8_t D_val;

        uint8_t serial_msg[SERIAL_MSG_DIM] = {'\0'};

        // Digital pin read
        D_val = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

        // Start the DMA conversion and pass the ADC instance
        AD_val = 0;
        HANDLE_HAL_ERROR(HAL_ADC_Start_DMA(hadc, &AD_val, 1));
        AD_val = HAL_ADC_GetValue(hadc);
        // Conversion Complete & DMA Transfer Complete As Well
        // So The AD_RES Is Now Updated & Let's Move IT To The PWM CCR1
        // Update The PWM Duty Cycle With Latest ADC Conversion Result
        TIM3->CCR1 = (AD_val << 4);

        // Moving average
        if (*cli_cmd == CLI_MOVING_AVERAGE) {
                uint32_t sum = 0;
                for (int i = 0; i < ANA_LOG_LEN - 1; ++i) {
                        sum += ana_log_arr[i];
                        ana_log_arr[i] = ana_log_arr[i + 1];
                }
                sum += AD_val;
                ana_log_arr[ANA_LOG_LEN - 1] = AD_val;
                AD_val = sum / ANA_LOG_LEN;
        }
        // Random noise
        else if (*cli_cmd == CLI_NOISE) {
                AD_val ^= AD_val << (AD_val % 4);
                AD_val += AD_val ^ (AD_val >> 1);
                if(AD_val < 500) AD_val += AD_val | (TIM3->ARR);
        }

        // Send data to serial

        // digital ch  |   analog ch
        // 00 00 00 00 | 00 00 00 10 ~
        // 00 00 00 01 | 00 00 0e 00 ~
        serial_msg[3] = D_val;

        for (int i = 0; i < 2; ++i)
                serial_msg[SERIAL_MSG_DIM - 1 - i] =
                    (uint8_t)(AD_val >> (i * 8));

        // Transmitting the msg to the uart, the data stream will be read
        // with SerialPlot as raw binary stream
        HANDLE_HAL_ERROR(
            HAL_UART_Transmit(huart, serial_msg, SERIAL_MSG_DIM, 0xFFFF));
}

void pause(enum FSM_State *state, enum CLI_STATE *cli_state) {
        // cli on
        *cli_state = CLI_ON;

        // On button press transition to listening
        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {
                *state = Listening;
                return;
        }

        // 2000ms = (ARR-1)(PSC-1)/(CLK) = (95999)(999)/(48MHz)
        TIM1->PSC = 999;
        TIM1->ARR = 95999;
        // Duty Cycle = 50% = ARR/2
        TIM1->CCR1 = TIM1->ARR / 2;

        // Led on only half of the period
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void warning(enum FSM_State *state, UART_HandleTypeDef *huart) {
        // Led off
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        // spam WARNING via serial
        uint8_t warning_msg[] = "WARNING\r\n";
        HAL_UART_Transmit(huart, warning_msg, sizeof(warning_msg), 100);
}

void error(enum FSM_State *state, UART_HandleTypeDef *huart) {
        // 400ms = (ARR-1)(PSC-1)/(CLK) = (19199)(999)/(48MHz)
        TIM1->PSC = 999;
        TIM1->ARR = 19199;
        // Duty Cycle = 50% = ARR/2
        TIM1->CCR1 = TIM1->ARR / 2;

        // Led on only half of the period
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

        // Spam ERROR via serial
        uint8_t error_msg[] = "ERROR\r\n";
        HAL_UART_Transmit(huart, error_msg, sizeof(error_msg), 100);
}
