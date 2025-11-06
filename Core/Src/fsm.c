#include "fsm.h"
#include "main.h"
#include "stdio.h"

void FSM(enum FSM_State *state, UART_HandleTypeDef *huart,
         ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim) {
        switch (*state) {
        case Init:
                init(state, hadc);
                break;

        case Wait_Request:
                wait_request(state);
                break;

        case Listening:
                listening(state, huart, hadc);
                break;

        case Pause:
                pause(state, htim);
                break;

        case Warning:
                warning(state, huart);
                break;

        case Error:
                error(state, huart, htim);
                break;
        }
}

void init(enum FSM_State *state, ADC_HandleTypeDef *hadc) {
        HANDLE_HAL_ERROR(HAL_ADCEx_Calibration_Start(hadc));

        *state = Wait_Request;
}

void wait_request(enum FSM_State *state) {
        // Led Pin Off
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        // On button press transition to listening
        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
                *state = Listening;
}

void listening(enum FSM_State *state, UART_HandleTypeDef *huart,
               ADC_HandleTypeDef *hadc) {
        // Led Pin On
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

        // On button press transition to pause
        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {
                *state = Pause;
                return;
        }
        HAL_StatusTypeDef HAL_status = HAL_OK;

        uint32_t AD_val;
        uint8_t D_val;

        uint8_t serial_msg[SERIAL_MSG_DIM] = {'\0'};

        // Digital pin read
        D_val = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

        // Start the DMA conversion and pass the ADC instance
        AD_val = 0;
        HANDLE_HAL_ERROR(HAL_ADC_Start_DMA(hadc, &AD_val, 1));
        AD_val = HAL_ADC_GetValue(hadc);

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

        // Conversion Complete & DMA Transfer Complete As Well
        // So The AD_RES Is Now Updated & Let's Move IT To The PWM CCR1
        // Update The PWM Duty Cycle With Latest ADC Conversion Result
        TIM3->CCR1 = (AD_val << 4);
}

void pause(enum FSM_State *state, TIM_HandleTypeDef *htim) {
        TIM14->PSC = 999;
        TIM14->ARR = 143999;
        if (TIM14->CNT < TIM14->ARR/2)
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_AF2_TIM14);
        else
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
                *state = Listening;
}

void warning(enum FSM_State *state, UART_HandleTypeDef *huart) {
        uint8_t warning_msg[] = "WARNING\r\n";
        HAL_UART_Transmit(huart, warning_msg, sizeof(warning_msg), 100);
}

void error(enum FSM_State *state, UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim) {
        // 400ms = (ARR-1)(PSC-1)/(CLK) = (28799)(999)/(72MHz)
        TIM14->PSC = 999;
        TIM14->ARR = 28799;
        // __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (uint32_t)(TIM14->ARR/2));
        uint8_t error_msg[] = "ERROR\r\n";

        TIM14->CCR1 = TIM14->ARR/2;
        // TIM14->AF2 = TIM14->CCR1;
        if (TIM14->CNT < TIM14->ARR/2)
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_AF2_TIM14);
        else
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        HAL_UART_Transmit(huart, error_msg, sizeof(error_msg), 100);
}
