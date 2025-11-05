#include "fsm.h"
#include "main.h"
#include "stdio.h"

void FSM(enum FSM_State *state, UART_HandleTypeDef *huart,
         ADC_HandleTypeDef *hadc) {
        switch (*state) {
        case Init:
                init(state);
                break;

        case Wait_Request:
                wait_request(state);
                break;

        case Listening:
                listening(state, huart, hadc);
                break;

        case Pause:
                pause(state);
                break;

        case Warning:
                warning(state, huart);
                break;

        case Error:
                error(state, huart);
                break;
        }
}

void init(enum FSM_State *state) { *state = Wait_Request; }

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

        uint32_t AD_RES;
        uint8_t D_State;

        // Digital pin read
        D_State = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

        // Start the DMA conversion and pass the ADC instance
        AD_RES = 0;
        HAL_ADC_Start_DMA(hadc, &AD_RES, 1);
        AD_RES = HAL_ADC_GetValue(hadc);

        // Send data to serial
        uint8_t AD_arr[4];
        for (int i = 0; i < 4; ++i) {
                AD_arr[3-i] = (AD_RES >> (i * 8));
        }
        uint8_t MSG[8] = {'\0'};

        MSG[3] = D_State;
        for(int i = 4; i < 8; ++i)
                MSG[i] = AD_arr[i-4];

        // sprintf(MSG, "%4c%4s", D_State, AD_arr);
        // for (int i = 0; i < 8; ++i)
        //         if (MSG[i] == ' ')
        //                 MSG[i] = '\0';
        HAL_UART_Transmit(huart, MSG, sizeof(MSG), 100);

        // Conversion Complete & DMA Transfer Complete As Well
        // So The AD_RES Is Now Updated & Let's Move IT To The PWM CCR1
        // Update The PWM Duty Cycle With Latest ADC Conversion Result
        TIM3->CCR1 = (AD_RES << 4);
}

void pause(enum FSM_State *state) {
        // Led Pin Off
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
                *state = Listening;
}

void warning(enum FSM_State *state, UART_HandleTypeDef *huart) {
        uint8_t MSG[] = "WARNING\r\n";
        HAL_UART_Transmit(huart, MSG, sizeof(MSG), 100);
}

void error(enum FSM_State *state, UART_HandleTypeDef *huart) {
        uint8_t MSG[] = "ERROR\r\n";
        HAL_UART_Transmit(huart, MSG, sizeof(MSG), 100);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(1000);
}
