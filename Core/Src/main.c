/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fsm.h"
#include "cli.h"
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_CMD_LEN 16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
enum CLI_CMD cli_cmd;
enum CLI_STATE cli_state;
uint8_t cmd_received = 0;
uint8_t cmd_buffer[MAX_CMD_LEN] = {0};
uint8_t *last_char = &cmd_buffer[MAX_CMD_LEN];
uint8_t *p_current_char = cmd_buffer;
uint32_t prev_millis = 0;
const uint32_t interval_ms = 500;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum FSM_State state = Init;
uint32_t ana_log_arr[ANA_LOG_LEN] = {0};
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

        /* USER CODE BEGIN 1 */

        /* USER CODE END 1 */

        /* MCU
         * Configuration--------------------------------------------------------*/

        /* Reset of all peripherals, Initializes the Flash interface and the
         * Systick. */
        HAL_Init();

        /* USER CODE BEGIN Init */

        /* USER CODE END Init */

        /* Configure the system clock */
        SystemClock_Config();

        /* USER CODE BEGIN SysInit */

        /* USER CODE END SysInit */

        /* Initialize all configured peripherals */
        MX_GPIO_Init();
        MX_DMA_Init();
        MX_USART2_UART_Init();
        MX_ADC1_Init();
        MX_TIM3_Init();
        MX_TIM14_Init();
        MX_TIM1_Init();
        MX_TIM16_Init();
        /* USER CODE BEGIN 2 */
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);

        HAL_UART_Receive_IT(&huart2, cmd_buffer, (size_t)1);

        /* USER CODE END 2 */

        /* Infinite loop */
        /* USER CODE BEGIN WHILE */
        while (1) {
                // Finite State Machine on every frame
                FSM(&state, &huart2,&cli_state, &cli_cmd, &hadc1, ana_log_arr);

                // CLI logic
                if (cli_state == CLI_ON && cmd_received) {
                        if (strcmp((char *)cmd_buffer, "raw") == 0)
                                cli_cmd = CLI_RAW;
                        else if (strcmp((char *)cmd_buffer, "avg") == 0)
                                cli_cmd = CLI_MOVING_AVERAGE;
                        else if (strcmp((char *)cmd_buffer, "noise") == 0)
                                cli_cmd = CLI_NOISE;

                        // Reset variables to receive another command.
                        memset(cmd_buffer, 0, strlen((char *)cmd_buffer));
                        p_current_char = cmd_buffer;
                        cmd_received = 0;
                        HAL_UART_Receive_IT(&huart2, cmd_buffer, (size_t)1);
                }
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

        __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

        /** Initializes the RCC Oscillators according to the specified
         * parameters in the RCC_OscInitTypeDef structure.
         */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
        RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
                Error_Handler();
        }

        /** Initializes the CPU, AHB and APB buses clocks
         */
        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
        RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) !=
            HAL_OK) {
                Error_Handler();
        }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

        /* USER CODE BEGIN ADC1_Init 0 */

        /* USER CODE END ADC1_Init 0 */

        ADC_ChannelConfTypeDef sConfig = {0};

        /* USER CODE BEGIN ADC1_Init 1 */

        /* USER CODE END ADC1_Init 1 */

        /** Configure the global features of the ADC (Clock, Resolution, Data
         * Alignment and number of conversion)
         */
        hadc1.Instance = ADC1;
        hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
        hadc1.Init.Resolution = ADC_RESOLUTION_12B;
        hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
        hadc1.Init.ScanConvMode = ADC_SCAN_SEQ_FIXED;
        hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
        hadc1.Init.LowPowerAutoWait = DISABLE;
        hadc1.Init.LowPowerAutoPowerOff = DISABLE;
        hadc1.Init.ContinuousConvMode = ENABLE;
        hadc1.Init.NbrOfConversion = 1;
        hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
        hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
        hadc1.Init.DMAContinuousRequests = ENABLE;
        hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
        hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
        hadc1.Init.OversamplingMode = DISABLE;
        hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
        if (HAL_ADC_Init(&hadc1) != HAL_OK) {
                Error_Handler();
        }

        /** Configure Regular Channel
         */
        sConfig.Channel = ADC_CHANNEL_18;
        sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN ADC1_Init 2 */

        /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

        /* USER CODE BEGIN TIM1_Init 0 */

        /* USER CODE END TIM1_Init 0 */

        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        TIM_OC_InitTypeDef sConfigOC = {0};
        TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        /* USER CODE BEGIN TIM1_Init 1 */

        /* USER CODE END TIM1_Init 1 */
        htim1.Instance = TIM1;
        htim1.Init.Prescaler = 999;
        htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim1.Init.Period = 28799;
        htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim1.Init.RepetitionCounter = 0;
        htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
                Error_Handler();
        }
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
                Error_Handler();
        }
        if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
                Error_Handler();
        }
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) !=
            HAL_OK) {
                Error_Handler();
        }
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = 0;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
        sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
        if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) !=
            HAL_OK) {
                Error_Handler();
        }
        sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
        sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
        sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
        sBreakDeadTimeConfig.DeadTime = 0;
        sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
        sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
        sBreakDeadTimeConfig.BreakFilter = 0;
        sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
        sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
        sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
        sBreakDeadTimeConfig.Break2Filter = 0;
        sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
        sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
        if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) !=
            HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN TIM1_Init 2 */

        /* USER CODE END TIM1_Init 2 */
        HAL_TIM_MspPostInit(&htim1);
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

        /* USER CODE BEGIN TIM3_Init 0 */

        /* USER CODE END TIM3_Init 0 */

        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        TIM_OC_InitTypeDef sConfigOC = {0};

        /* USER CODE BEGIN TIM3_Init 1 */

        /* USER CODE END TIM3_Init 1 */
        htim3.Instance = TIM3;
        htim3.Init.Prescaler = 0;
        htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim3.Init.Period = 65535;
        htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
                Error_Handler();
        }
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
                Error_Handler();
        }
        if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
                Error_Handler();
        }
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) !=
            HAL_OK) {
                Error_Handler();
        }
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = 0;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) !=
            HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN TIM3_Init 2 */

        /* USER CODE END TIM3_Init 2 */
        HAL_TIM_MspPostInit(&htim3);
}

/**
 * @brief TIM14 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM14_Init(void) {

        /* USER CODE BEGIN TIM14_Init 0 */

        /* USER CODE END TIM14_Init 0 */

        TIM_OC_InitTypeDef sConfigOC = {0};

        /* USER CODE BEGIN TIM14_Init 1 */

        /* USER CODE END TIM14_Init 1 */
        htim14.Instance = TIM14;
        htim14.Init.Prescaler = 999;
        htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim14.Init.Period = 4799;
        htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        if (HAL_TIM_Base_Init(&htim14) != HAL_OK) {
                Error_Handler();
        }
        if (HAL_TIM_PWM_Init(&htim14) != HAL_OK) {
                Error_Handler();
        }
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = 0;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) !=
            HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN TIM14_Init 2 */

        /* USER CODE END TIM14_Init 2 */
        HAL_TIM_MspPostInit(&htim14);
}

/**
 * @brief TIM16 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM16_Init(void) {

        /* USER CODE BEGIN TIM16_Init 0 */

        /* USER CODE END TIM16_Init 0 */

        /* USER CODE BEGIN TIM16_Init 1 */

        /* USER CODE END TIM16_Init 1 */
        htim16.Instance = TIM16;
        htim16.Init.Prescaler = 0;
        htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim16.Init.Period = 65535;
        htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim16.Init.RepetitionCounter = 0;
        htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&htim16) != HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN TIM16_Init 2 */

        /* USER CODE END TIM16_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

        /* USER CODE BEGIN USART2_Init 0 */

        /* USER CODE END USART2_Init 0 */

        /* USER CODE BEGIN USART2_Init 1 */

        /* USER CODE END USART2_Init 1 */
        huart2.Instance = USART2;
        huart2.Init.BaudRate = 9600;
        huart2.Init.WordLength = UART_WORDLENGTH_8B;
        huart2.Init.StopBits = UART_STOPBITS_1;
        huart2.Init.Parity = UART_PARITY_NONE;
        huart2.Init.Mode = UART_MODE_TX_RX;
        huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart2.Init.OverSampling = UART_OVERSAMPLING_16;
        huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
        huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
        if (HAL_UART_Init(&huart2) != HAL_OK) {
                Error_Handler();
        }
        /* USER CODE BEGIN USART2_Init 2 */

        /* USER CODE END USART2_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

        /* DMA controller clock enable */
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* DMA interrupt init */
        /* DMA1_Channel1_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
        /* DMA1_Channel2_3_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        /* USER CODE BEGIN MX_GPIO_Init_1 */

        /* USER CODE END MX_GPIO_Init_1 */

        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        /*Configure GPIO pin : PC13 */
        GPIO_InitStruct.Pin = GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /*Configure GPIO pin : PA4 */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /*Configure GPIO pin : PA5 */
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_TIM1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USER CODE BEGIN MX_GPIO_Init_2 */

        /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
        // As long as the character received isn't a newline/carriage return AND
        // we haven't reached the end of our cmd buffer, then restart the
        // interrupt to receive another character.
        //
        if (*p_current_char != '\n' && *p_current_char != '\r' &&
            p_current_char != last_char) {
                HAL_UART_Receive_IT(huart, ++p_current_char, (size_t)1);
        } else {
                *p_current_char = '\0';
                cmd_received = 1;
        }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
        // if (htim->Instance == TIM14) {
        //         if (state == Error)
        //                 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        //         HAL_UART_Transmit(&huart2, "hello\r\n", 12, 1000);
        // }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
        /* USER CODE BEGIN Error_Handler_Debug */
        /* User can add his own implementation to report the HAL error return
         * state */
        __disable_irq();
        while (1) {
        }
        /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
        /* USER CODE BEGIN 6 */
        /* User can add his own implementation to report the file name and line
           number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
           file, line) */
        /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
