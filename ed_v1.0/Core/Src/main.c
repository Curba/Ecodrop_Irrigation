/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.cIGULI
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc_conversion_handler.h"
#include "lcd16x2_i2c.h"
#include "i2c-lcd.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "cJSON.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* ADC Variables */
volatile uint16_t adc_result_dma[7];
const int adc_channel_lenght = sizeof(adc_result_dma) / sizeof(adc_result_dma[0]);
int adc_result_percentage[7];
const int moist_per_line[] = {2,2};
const int adc_line_number = 2;
const int adc_moist_number = 4;
int adc_result_line_avg_percentage = 0;
int adc_line_avg[2] = {0,0};
int adc_total_moist_avg = 0;

char adc_percentage_s0[5];
char adc_percentage_s1[5];
char adc_percentage_s2[5];
char adc_percentage_s3[5];


/* Menu Variable */
int menu_lcd_refresh = 0;
int menu_click = 0;
int menu_up = 0;
int menu_down = 0;
int next_page=0;
int current_page = 0;
int menu_refresher = 0;

/* Manual Mode Variables */
int mode_manual_start = 0;

/* Planner Mode Variables */
int planner_line_compare[2] = {50,80};
int planner_line_lenght = sizeof(planner_line_compare) /sizeof(planner_line_compare[0]);
int relay_set[2];
int relay_end_flag = 0;
int line_relays[] = {line0_relay_Pin, line1_relay_Pin};
int global_planner_start = 0;
int global_planner_active = 1;

int planner_start_time_hours = 17; //0-24
int planner_start_time_mins = 30; //0-60
int planner_weekdays [7] = {0,0,0,0,0,0,0};

/* Auto Mode Variables ------------------------------------------*/
int auto_modes[2][4][9] = {{{1,20,0,0,0,1,0,0,0}, {2,20,0,1,0,0,0,1,0}, {2,30,0,1,0,0,1,0,0}, {2,60,0,1,0,0,1,0,0}},
							{{1,30,0,0,0,1,0,0,0}, {2,30,0,1,0,0,0,1,0}, {3,60,0,1,0,1,0,1,0}, {3,120,0,1,0,1,0,1,0}}};

// 0:High Temp | 1:Low Temp | 2:High Moist | 3:Low Moist
int auto_limits[4] = {35,20,60,40};

// 0:Sapling | 1:Tree
int auto_irrigation_time[2];

/*	Index = 0:Sapling | 1:Tree
	Value = 0:Low | 1:Mid | 2:High */
int auto_mode_param[2];

int auto_start_time_hours = 18; //2400 format
int auto_start_time_mins = 0;
int auto_avg_temp = 0;
int auto_avg_airhum = 0;
int auto_week_skip_vector[7] = {0,0,0,0,0,0,0};
int week_reset_flag = 0;
int reset_day = 0;
/* Auto Mode Variables End------------------------------------------*/

/* Real Time Clock */
RTC_TimeTypeDef currTime = {0};
RTC_DateTypeDef currDate = {0};
char timeBuff[20];
char dateBuff[20];
int set_hours = 0; 		int set_minutes = 0; int set_seconds = 0;
int set_weekday = 0;  	int set_month = 0;   int set_date = 0;
int set_year = 0;

RTC_AlarmTypeDef sAlarmA = {0};
RTC_AlarmTypeDef sAlarmB = {0};
int current_hour = 0;
int current_min = 0;
int current_wday = 0;

/* DHT Temp Sensor Variables */
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;

float inst_temperature = 0;
float inst_humidity = 0;
uint8_t Presence = 0;
char inst_temperature_s[5];
char inst_humidity_s[5];


/* ESP32-UART Variables */

char str_tempJSON[1000], str_humJSON[1000], str_flowJSON[1000], str_s1JSON[1000], str_s2JSON[1000];
char str_s3JSON[1000], str_s4JSON[800], str_rainJSON[800], str_daylightJSON[800], str_tankJSON[1000];
char str_modJSON[1000], str_relaytankJSON[800], str_relaywellJSON[1000], str_relayline1JSON[1000];
char str_relayline2JSON[1000], str_donewateringJSON[1000];
char str_debughourJSON[1000];
char str_debugminuteJSON[1000];
char str_debugskipdayJSON[1000];

int wellrelay = 0;
int tankrelay = 0;
int line1relay = 0;
int line2relay = 0;
int donewatering = 0;
int rain = 0;
int daylight = 0;
int current_mode = 0;
int waterflow = 0;
int auto_select = 0;

char JSON[1200];
char rx_buffer[200];

uint8_t rx_data = 0;
unsigned int rx_index = 0;

cJSON *str_json, *str_c_tempJSON;
long last = 0;

/* DEBUG Variables */
int yanked;
int x =0;
int a =5;
int b=8;
int testDummy_manual = 0;
int nextday_dummy = 0;
int mode_dummy = 0;
int debug_hours = 0;
int debug_mins = 0;
int auto_debug_weekday = 0;
int debug_auto_case =0;
int skipday = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */


//MENU
void menu_func(int refresh, int reset);
void lcd_initial_message();

//MODES
void mode_manual(int start);
void mode_planner(int auto_current_mode, int start);
void mode_auto();

//DELAY
void delay_us (uint16_t us);

//DHT PROTOTYPES
void DHT11_Start (void);
uint8_t DHT11_Check_Response (void);
uint8_t DHT11_Read (void);
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void DHT11_get_value();

//RTC PROTOTYPES
void set_time (int set_hours, int set_minutes, int set_weekday);
void get_time();
void set_alarm(int hours_hex, int mins_hex, int weekday_hex);
int bcdToDec(int val);
int decToBcd(int val);


//ESP32 Connections
void SendData(unsigned int tempJSON, unsigned int humJSON, unsigned int flowJSON, unsigned int s1JSON, unsigned int s2JSON,
		unsigned int s3JSON, unsigned int s4JSON, unsigned int rainJSON, unsigned int daylightJSON, unsigned int tankJSON,
		unsigned int modJSON, unsigned int relaytankJSON, unsigned int relaywellJSON, unsigned int relayline1JSON,
		unsigned int relayline2JSON, unsigned int donewateringJSON,
		unsigned int debughourJSON, unsigned int debugminuteJSON, unsigned int debugskipdayJSON);

void uart_received(void);
void clearbufferEnd(void);
void XuLyJson(char *DataJson);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */


  HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, -511);
  HAL_TIM_Base_Start(&htim1);
  lcd_initial_message();
  HAL_UART_Receive_IT(&huart6, &rx_data, 1);
  last = HAL_GetTick();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_Delay(500);
	  DHT11_get_value();
	  HAL_Delay(900);
	  get_time();
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_result_dma, adc_channel_lenght);
	  if(menu_refresher == 10){
		  menu_lcd_refresh = 1;
		  menu_refresher = 0;
	  }else{
		  menu_refresher++;
	  }

	  if(HAL_GetTick() - last >= 1500){
	  		  SendData(inst_temperature, inst_humidity, waterflow ,adc_result_percentage[0],
	  				  adc_result_percentage[1],adc_result_percentage[2], adc_result_percentage[3], rain, daylight,
					  adc_result_percentage[5], current_mode, tankrelay, wellrelay, line1relay, line2relay,
					  donewatering, current_hour, current_min, skipday);
	  		  last = HAL_GetTick();
	  	  }
	  mode_planner(0, global_planner_start);
	  menu_func(menu_lcd_refresh, 0);
	  HAL_Delay(100);

	  if(nextday_dummy){
		  set_time (decToBcd(17), decToBcd(30), auto_debug_weekday);
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 7;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 285;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x17;
  sTime.Minutes = 0x30;
  sTime.Seconds = 0x10;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JUNE;
  sDate.Date = 0x13;
  sDate.Year = 0x22;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x17;
  sAlarm.AlarmTime.Minutes = 0x30;
  sAlarm.AlarmTime.Seconds = 0x15;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
  sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_WEDNESDAY;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 84-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, tankmotor_relay_Pin|line1_relay_Pin|line0_relay_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(wellmotor_relay_GPIO_Port, wellmotor_relay_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : tankmotor_relay_Pin line1_relay_Pin line0_relay_Pin */
  GPIO_InitStruct.Pin = tankmotor_relay_Pin|line1_relay_Pin|line0_relay_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : light_exti_sens_Pin button_press_Pin */
  GPIO_InitStruct.Pin = light_exti_sens_Pin|button_press_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : button_up_Pin button_down_Pin */
  GPIO_InitStruct.Pin = button_up_Pin|button_down_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : wellmotor_relay_Pin */
  GPIO_InitStruct.Pin = wellmotor_relay_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(wellmotor_relay_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ac_source_Pin */
  GPIO_InitStruct.Pin = ac_source_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ac_source_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : dht_in_Pin */
  GPIO_InitStruct.Pin = dht_in_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(dht_in_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}


/*-----------------INTERRUPTS BEGIN-----------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc == &hadc1){
		for(int x =0; x<adc_moist_number-1; x++){
			adc_result_dma[x] = (adc_result_dma[x]> 2730) ? 2730:adc_result_dma[x];
			adc_result_dma[x] = (adc_result_dma[x]< 1260) ? 1260:adc_result_dma[x];
			adc_result_percentage[x] = 100-((adc_result_dma[x]-1260)*100/1470);
		}
		//Different Moisture Sensor
		adc_result_dma[3] = (adc_result_dma[3]> 3800) ? 3800:adc_result_dma[3];
		adc_result_dma[3] = (adc_result_dma[3]< 900) ? 900:adc_result_dma[3];
		adc_result_percentage[3] = 100-((adc_result_dma[3]-900)*100/2900);

		adc_result_percentage[5] =100-((adc_result_dma[5])*100/4000);

		//Water Tank Refill Algorithm
		if(adc_result_dma[5] < 1800){
			HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_RESET);
			wellrelay = 1;
		}else{
			HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_SET);
			wellrelay = 0;
		}
		rain = (adc_result_dma[4] < 2600) ? 1:0;
		daylight = (adc_result_dma[4] > 3000) ? 1:0;
		waterflow = ((adc_result_dma[6])*100/4000);

		//Produces Line and Total Average : adc_line_avg and adc_total_moist_avg
		adc_total_moist_avg = 0;
		int y =0;
		int per_line_offset = moist_per_line[0];
		for(int x = 0; x < adc_line_number; x++){
			adc_line_avg[x] = 0;
			for(; y < per_line_offset; y++){
				adc_line_avg[x] += adc_result_percentage[y];
			}
			adc_line_avg[x] /= moist_per_line[x];
		   per_line_offset += moist_per_line[x+1];
		   adc_total_moist_avg += adc_line_avg[x];
		}
		adc_total_moist_avg /= adc_line_number;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	menu_lcd_refresh = 1;
    if(GPIO_Pin == button_up_Pin){
    	menu_up = 1;
    }else if(GPIO_Pin == button_down_Pin){
    	menu_down = 1;
    }else if(GPIO_Pin == button_press_Pin){
    	menu_click = 1;
    }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(global_planner_active){
		global_planner_start = 1;
		mode_planner(0, global_planner_start);
	}
}


/*-----------------INTERRUPTS END-----------------------*/


/*-----------------DHT11 BEGIN--------------------------*/
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11_Start (void)
{
	Set_Pin_Output (DHT11_PORT, DHT11_PIN);  // set the pin as output
	HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);   // pull the pin low
	delay_us (18000);   // wait for 18ms
    HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);   // pull the pin high
    delay_us (20);   // wait for 20us
	Set_Pin_Input(DHT11_PORT, DHT11_PIN);    // set as input
}

uint8_t DHT11_Check_Response (void)
{
	uint8_t Response = 0;
	delay_us (40);
	if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
	{
		delay_us (80);
		if ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN))) Response = 1;
		else Response = -1; // 255
	}
	while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));   // wait for the pin to go low

	return Response;
}

uint8_t DHT11_Read (void)
{
	uint8_t i,j;
	for (j=0;j<8;j++)
	{
		while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));   // wait for the pin to go high
		delay_us (40);   // wait for 40 us
		if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));  // wait for the pin to go low
	}
	return i;
}

void DHT11_get_value(){
	DHT11_Start();
	Presence = DHT11_Check_Response();
	Rh_byte1 = DHT11_Read ();
	Rh_byte2 = DHT11_Read ();
	Temp_byte1 = DHT11_Read ();
	Temp_byte2 = DHT11_Read ();
	SUM = DHT11_Read();
	TEMP = Temp_byte1;
	RH = Rh_byte1;
	inst_temperature = (float) TEMP;
	inst_humidity = (float) RH;
}

/*-----------------DHT11 END----------------------------*/

/*-----------------MODE FUNCTIONS BEGIN-----------------*/
void mode_manual(int start){
	if(start){
		  current_mode = 1;
		  donewatering = 0;
		  HAL_GPIO_WritePin(GPIOC, tankmotor_relay_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_RESET);
		  wellrelay = 1;
		  tankrelay = 1;
		  delay_us (5000);
		  HAL_GPIO_WritePin(GPIOC, line0_relay_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, line1_relay_Pin, GPIO_PIN_RESET);
		  line1relay = 1;
		  line2relay = 1;
	}else{
		current_mode = 0;
		donewatering = 1;
		  HAL_GPIO_WritePin(GPIOC, tankmotor_relay_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_SET);
		  wellrelay = 0;
		  tankrelay = 0;
		  delay_us (5000);
		  HAL_GPIO_WritePin(GPIOC, line0_relay_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOC, line1_relay_Pin, GPIO_PIN_SET);
		  line1relay = 0;
		  line2relay = 0;
	}
}

void mode_planner(int auto_current_mode, int start){
	if(auto_current_mode == 0 && start == 1){
		current_mode = 2;
		donewatering = 0;
		HAL_GPIO_WritePin(GPIOC, tankmotor_relay_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_RESET);
		wellrelay = 1;
		tankrelay = 1;
		delay_us(1000);
		HAL_GPIO_WritePin(GPIOC, line0_relay_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, line1_relay_Pin, GPIO_PIN_RESET);
		line1relay = 1;
		line2relay = 1;
		for(int x = 0; x < adc_line_number; x++){
			//for(int x = 0; x < moist_per_line[a]; x++){
			if(adc_line_avg[x] >= planner_line_compare[x]){
				relay_set[x] = 1;	//Debug
				relay_end_flag = (relay_end_flag == 2) ? 2:relay_end_flag+1;
				HAL_GPIO_WritePin(GPIOC, line_relays[x], GPIO_PIN_SET);
				line1relay = (x) ? 1:0;
				line2relay = (x) ? 0:1;
			}else{
				relay_set[x] = 0;	//Debug
				relay_end_flag = (relay_end_flag) ? relay_end_flag-1:0;
				HAL_GPIO_WritePin(GPIOC, line_relays[x], GPIO_PIN_RESET);
				line1relay = (x) ? 0:1;
				line2relay = (x) ? 1:0;
			}
			//}
		}
		if(relay_end_flag == 2){
			for(int a = 0; a < 2; a++){
				relay_set[a] = 0;	//Debug
			}
			donewatering = 1;
			global_planner_start = 0;
			relay_end_flag = 0;
			HAL_GPIO_WritePin(GPIOC, tankmotor_relay_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, wellmotor_relay_Pin, GPIO_PIN_SET);
			wellrelay = 0;
			tankrelay = 0;
			delay_us(1000);
			HAL_GPIO_WritePin(GPIOC, line0_relay_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, line1_relay_Pin, GPIO_PIN_SET);
			line1relay = 0;
			line2relay = 0;
			current_mode = 0;

			//Alarm for the Next Day
			for(int x = bcdToDec(current_wday-1); x <7; x++){
				if(planner_weekdays[x] == 1){
					set_alarm(decToBcd(planner_start_time_hours), decToBcd(planner_start_time_mins), x+1);
					break;
				}
			}
		}
	}
}

void mode_auto(int start){
/*
//Saat geldi mi
	//yagisli mi ya da genel toprak nemi >80
	//Gun atlandi mi
		//evet: her modu ertesi gune aktar
			//bu hafta 2. kez yagisliysa sulama yapma bu hafta

		//hayir:
			//sapling + agac
				//hafta ortalamalari hangi moda denk geliyor
					//o modun sulama gunu bugun mu
						//evet: moda gore sula
						///hayir: skip
*/

//Function is called with alarm
	current_mode = 3;
	//int auto_hours = 0;
	//int auto_mins = 0;
//	int auto_secs = 0;

	if(adc_result_dma[4] > 2000 || adc_total_moist_avg > 80 || debug_auto_case == 4){
		//Skip Day
		auto_week_skip_vector[currDate.WeekDay-1] = 1;
		skipday = 1;

		if(week_reset_flag){
			if(currDate.WeekDay == reset_day){
				week_reset_flag = 1;
			}
		}else{
			//Repeated: Skip Rest of the Week
			if(auto_week_skip_vector[(currDate.WeekDay == 1) ? 6:currDate.WeekDay-2] == 1){ //One day before
				reset_day = currDate.WeekDay;
				week_reset_flag = 1;
				for(int x = 0; x <7; x ++){
					skipday = 2;
					auto_week_skip_vector[x] = 1;
				}
			}
		}
	}else{
		if(week_reset_flag){
			if(currDate.WeekDay == reset_day){
				week_reset_flag = 0;
				for(int x = 0; x <7; x ++){
					auto_week_skip_vector[x] = 0;
				}
			}
		}else{
			auto_week_skip_vector[currDate.WeekDay-1] = 0;
		}
	}

	//Day Skipped?
	if(auto_week_skip_vector[(currDate.WeekDay)-1] == 0){
		//Rain or >80 humidity (Just Rained)
		if(inst_humidity > 80 || adc_result_dma[4] > 3000){
			auto_week_skip_vector[(currDate.WeekDay)-1] = 1;
		}
		//else{
			/*for(int x = 0; x < adc_line_number; x++){
				if(auto_avg_temp > auto_limits[0] && auto_avg_airhum < auto_limits[3]){
					//High
					if(auto_modes[x][3][(currDate.WeekDay)+1] == 1){
						auto_select = 0;
						//auto_hours = auto_modes[x][3][1]/60;
						//auto_hours = (auto_hours < 1) ? 0: auto_hours;
						//currTime.Hours, currTime.Minutes, currTime.Seconds
						//void set_time (int set_hours, int set_minutes, int set_seconds, int set_weekday, int set_month, int set_date, int set_year)
						//auto_modes[x][3][1]dk timer ya da alarm
					}
				}else if(auto_avg_temp > auto_limits[0] && auto_avg_airhum < auto_limits[2]){
					//Mid
					if(auto_modes[x][2][(currDate.WeekDay)+1] == 1){
						auto_select = 1;
						//auto_modes[x][2][1]dk timer ya da alarm
					}
				}else if(auto_avg_temp > auto_limits[0]){
					//Low
					if(auto_modes[x][1][(currDate.WeekDay)+1] == 1){
						auto_select = 2;
						//auto_modes[x][1][1]dk timer ya da alarm
					}
				}else if(auto_avg_temp > auto_limits[1]){
					//Min
					if(auto_modes[x][1][(currDate.WeekDay)+1] == 1){
						auto_select = 3;
						//auto_modes[x][1][1]dk timer ya da alarm
					}

				}else{
					//Skip Day
					auto_week_skip_vector[(currDate.WeekDay)-1] = 1;
				}
			}
		}*/
	}
}

/*-----------------MODE FUNCTIONS END-----------------*/


/*-----------------RTC FUNCTIONS BEGIN-----------------*/

int bcdToDec(int val)
{
  return( (val/16*10) + (val%16) );
}

int decToBcd(int val)
{
  return( (val/10*16) + (val%10) );
}

void set_time (int set_hours, int set_minutes, int set_weekday){
	  RTC_TimeTypeDef sTime;
	  RTC_DateTypeDef sDate;
	  sTime.Hours = set_hours;
	  sTime.Minutes = set_minutes;
	  sTime.Seconds = 0x00;
	  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sDate.WeekDay = set_weekday;
	  sDate.Month = RTC_MONTH_JUNE;
	  sDate.Date = 0x13;
	  sDate.Year = 0x22;

	  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void set_alarm(int hours_hex, int mins_hex, int weekday_hex){
/* 	RTC_AlarmTypeDef sAlarm = {0};

	sAlarm.AlarmTime.Hours = hours_hex;
	sAlarm.AlarmTime.Minutes = mins_hex;
	sAlarm.AlarmTime.Seconds = 0x15;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmDateWeekDay = weekday_hex;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	{
	Error_Handler();
	}*/

	  sAlarmA.AlarmTime.Hours = hours_hex;
	  sAlarmA.AlarmTime.Minutes = mins_hex;
	  sAlarmA.AlarmTime.Seconds = 0x15;
	  sAlarmA.AlarmTime.SubSeconds = 0x0;
	  sAlarmA.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sAlarmA.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  sAlarmA.AlarmMask = RTC_ALARMMASK_NONE;
	  sAlarmA.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  sAlarmA.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	  sAlarmA.AlarmDateWeekDay = weekday_hex;
	  sAlarmA.Alarm = RTC_ALARM_A;
	  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarmA, RTC_FORMAT_BCD) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void get_time(){
	HAL_RTC_GetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &currDate, RTC_FORMAT_BIN);
	sprintf(timeBuff,"%d,%d.%d", currTime.Hours, currTime.Minutes, currTime.Seconds);
	sprintf(dateBuff,"%02d-%02d-%2d",currDate.Date, currDate.Month, 2000 + currDate.Year);
	current_hour = currTime.Hours;
	current_min = currTime.Minutes;
	current_wday = currDate.WeekDay;
}

/*-----------------RTC FUNCTIONS END-----------------*/

/*-----------------ESP32 UART BEGIN------------------*/
void SendData(unsigned int tempJSON, unsigned int humJSON, unsigned int flowJSON, unsigned int s1JSON, unsigned int s2JSON,
		unsigned int s3JSON, unsigned int s4JSON, unsigned int rainJSON, unsigned int daylightJSON, unsigned int tankJSON,
		unsigned int modJSON, unsigned int relaytankJSON, unsigned int relaywellJSON, unsigned int relayline1JSON,
		unsigned int relayline2JSON, unsigned int donewateringJSON, unsigned int debughourJSON, unsigned int debugminuteJSON,
		unsigned int debugskipdayJSON){

	for(int i = 0; i < 1200; i++){
			JSON[i] = 0;
		}

	for(int i = 0; i < 1000; i++){
		str_tempJSON[i] = 0;
		str_humJSON[i] = 0;
		str_flowJSON[i] = 0;
		str_s1JSON[i] = 0;
		str_s2JSON[i] = 0;
		str_s3JSON[i] = 0;
		str_s4JSON[i] = 0;
		str_rainJSON[i] = 0;
		str_daylightJSON[i] = 0;
		str_tankJSON[i] = 0;
		str_modJSON[i] = 0;
		str_relaytankJSON[i] = 0;
		str_relaywellJSON[i] = 0;
		str_relayline1JSON[i] = 0;
		str_relayline2JSON[i] = 0;
		str_donewateringJSON[i] = 0;
		str_debughourJSON[i] = 0;
		str_debugminuteJSON[i] = 0;
		JSON[i] = 0;
	}

	sprintf(str_tempJSON, "%d", tempJSON);
	sprintf(str_humJSON, "%d", humJSON);
	sprintf(str_flowJSON, "%d", flowJSON);
	sprintf(str_s1JSON, "%d", s1JSON);
	sprintf(str_s2JSON, "%d", s2JSON);
	sprintf(str_s3JSON, "%d", s3JSON);
	sprintf(str_s4JSON, "%d", s4JSON);
	sprintf(str_rainJSON, "%d", rainJSON);
	sprintf(str_daylightJSON, "%d", daylightJSON);
	sprintf(str_tankJSON, "%d", tankJSON);
	sprintf(str_modJSON, "%d", modJSON);
	sprintf(str_relaytankJSON, "%d", relaytankJSON);
	sprintf(str_relaywellJSON, "%d", relaywellJSON);
	sprintf(str_relayline1JSON, "%d", relayline1JSON);
	sprintf(str_relayline2JSON, "%d", relayline2JSON);
	sprintf(str_donewateringJSON, "%d", donewateringJSON);
	sprintf(str_debughourJSON, "%d", debughourJSON);
	sprintf(str_debugminuteJSON, "%d", debugminuteJSON);
	sprintf(str_debugskipdayJSON, "%d", debugskipdayJSON);

	strcat(JSON, "{\"temperature\":\"");
	strcat(JSON, str_tempJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"humidity\":\"");
	strcat(JSON, str_humJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"flowrate\":\"");
	strcat(JSON, str_flowJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"s1\":\"");
	strcat(JSON, str_s1JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"s2\":\"");
	strcat(JSON, str_s2JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"s3\":\"");
	strcat(JSON, str_s3JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"s4\":\"");
	strcat(JSON, str_s4JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"rain\":\"");
	strcat(JSON, str_rainJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"daylight\":\"");
	strcat(JSON, str_daylightJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"tank\":\"");
	strcat(JSON, str_tankJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"mod\":\"");
	strcat(JSON, str_modJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"relaytank\":\"");
	strcat(JSON, str_relaytankJSON);
	//relay_set
	strcat(JSON, "\",");

	strcat(JSON, "\"relaywell\":\"");
	strcat(JSON, str_relaywellJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"relayline1\":\"");
	strcat(JSON, str_relayline1JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"relayline2\":\"");
	strcat(JSON, str_relayline2JSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"debughour\":\"");
	strcat(JSON, str_debughourJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"debugminute\":\"");
	strcat(JSON, str_debugminuteJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"skipday\":\"");
	strcat(JSON, str_debugskipdayJSON);
	strcat(JSON, "\",");

	strcat(JSON, "\"donewatering\":\"");
	strcat(JSON, str_donewateringJSON);
	strcat(JSON, "\"}\n");

	//HAL_Delay(100);
	//printf("%s", JSON);
	HAL_UART_Transmit(&huart6,(uint8_t *)&JSON , strlen(JSON), 0xFFFF);


}

void uart_received(void){
	if(rx_data != '\n'){
		rx_buffer[rx_index++] = rx_data;
	}else{
		XuLyJson(rx_buffer);
		clearbufferEnd();
	}
}

void clearbufferEnd(void){
	rx_index = 0;
	for(int i =0; i <200; i++){
		rx_buffer[i] = 0;
	}
	last = HAL_GetTick();
}

void XuLyJson(char *DataJson){
	str_json = cJSON_Parse(DataJson);
	if(!str_json){
		printf("JSON ERROR! \r\n");
		return;
	}else{
		printf("JSON OK! \r\n");
		if(cJSON_GetObjectItem(str_json, "reqline1")){
			planner_line_compare[0] = cJSON_GetObjectItem(str_json, "reqline1")->valueint;
		}if(cJSON_GetObjectItem(str_json, "reqline2")){
			planner_line_compare[1] = cJSON_GetObjectItem(str_json, "reqline2")->valueint;
		}if(cJSON_GetObjectItem(str_json, "hour")){
			planner_start_time_hours = cJSON_GetObjectItem(str_json, "hour")->valueint;
		}if(cJSON_GetObjectItem(str_json, "minute")){
			planner_start_time_mins = cJSON_GetObjectItem(str_json, "minute")->valueint;
		}if(cJSON_GetObjectItem(str_json, "mon")){
			planner_weekdays[0] = (cJSON_GetObjectItem(str_json, "mon")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "tue")){
			planner_weekdays[1] = (cJSON_GetObjectItem(str_json, "tue")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "wed")){
			planner_weekdays[2] = (cJSON_GetObjectItem(str_json, "wed")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "thu")){
			planner_weekdays[3] = (cJSON_GetObjectItem(str_json, "thu")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "fri")){
			planner_weekdays[4] = (cJSON_GetObjectItem(str_json, "fri")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "sat")){
			planner_weekdays[5] = (cJSON_GetObjectItem(str_json, "sat")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "sun")){
			planner_weekdays[6] = (cJSON_GetObjectItem(str_json, "sun")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "temphigh")){
			auto_limits[0] = cJSON_GetObjectItem(str_json, "temphigh")->valueint;
		}if(cJSON_GetObjectItem(str_json, "templow")){
			auto_limits[1] = cJSON_GetObjectItem(str_json, "templow")->valueint;
		}if(cJSON_GetObjectItem(str_json, "humhigh")){
			auto_limits[2] = cJSON_GetObjectItem(str_json, "humhigh")->valueint;
		}if(cJSON_GetObjectItem(str_json, "humlow")){
			auto_limits[3] = cJSON_GetObjectItem(str_json, "humlow")->valueint;
		}if(cJSON_GetObjectItem(str_json, "debugweek")){
			auto_debug_weekday = cJSON_GetObjectItem(str_json, "debugweek")->valueint;
		}if(cJSON_GetObjectItem(str_json, "debugon")){
			nextday_dummy = (cJSON_GetObjectItem(str_json, "debugon")->valueint) ? 1:0;
		}if(cJSON_GetObjectItem(str_json, "debugswitch")){
			debug_auto_case = cJSON_GetObjectItem(str_json, "debugswitch")->valueint;
		}if(cJSON_GetObjectItem(str_json, "modselect")){
			switch(cJSON_GetObjectItem(str_json, "modselect")->valueint){
				case 0://off
					mode_manual(0);
					mode_dummy = 0;
					skipday = 0;
					global_planner_start = 0;
					break;
				case 1://manual
					mode_manual(1);
					mode_dummy = 1;
					break;
				case 2://planner
					global_planner_start = 1;
					mode_dummy = 2;
					mode_planner(0, global_planner_start);
					break;
				case 3://auto
					mode_auto(1);
					break;
			}
		}
		cJSON_Delete(str_json);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart -> Instance == USART6){
		last = HAL_GetTick();
		uart_received();
		HAL_UART_Receive_IT(&huart6, &rx_data, 1);
	}
}
/*-----------------ESP32 UART END---------------------*/


/*-----------------MENU BEGIN-------------------------*/
void lcd_initial_message(){
	lcd_init ();
	lcd_send_string ("ECODROP");
	HAL_Delay(500);
	lcd_put_cur(1, 0);
	lcd_send_string("Irrigation");
	HAL_Delay(1000);
	lcd_send_cmd (0x01);
	HAL_Delay(100);
	lcd_init ();
	lcd_send_string ("> Manual Mode");
	lcd_put_cur(1, 0);
	lcd_send_string("  Planner Mode  ");

}

void menu_func(int refresh, int reset){
	if(refresh){
		current_page = (reset) ? 0:current_page;
		lcd_clear();
		switch(current_page){
			case 0:
				if(menu_click){
					current_page = 4;
					menu_click = 0;
					menu_func(1, 0);
					break;
				  }else if(menu_down){
					 current_page = 1;
					 menu_down=0;
					 menu_func(1, 0);
					 break;
				  }else if(menu_up){
					 current_page = 3;
					 menu_up =0;
					 menu_func(1, 0);
					 break;
				  }
				lcd_init ();
				lcd_send_string ("> Manual Mode");
				lcd_put_cur(1, 0);
				lcd_send_string("  Planner Mode  ");
				break;

			case 1:
				if(menu_click){
					current_page = 7;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 2;
					menu_down =0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 0;
					menu_up =0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Planner Mode  ");
				lcd_put_cur(1, 0);
				lcd_send_string("  Auto Mode");
				break;

			case 2:
				if(menu_click){
					current_page = 14;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 3;
					menu_down =0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 1;
					menu_up =0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Auto Mode");
				lcd_put_cur(1, 0);
				lcd_send_string("  Settings");
				break;

			case 3:
				if(menu_click){
					current_page = 15;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 0;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 2;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Settings");
				lcd_put_cur(1, 0);
				lcd_send_string("  ");
				break;

			case 4:
				if(menu_click){
					current_page = 6;
					menu_click = 0;
					mode_manual(1);
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 5;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 5;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Press Start");
				lcd_put_cur(1, 0);
				lcd_send_string(" Back");
				break;

			case 5:
				if(menu_click){
					current_page = 0;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 4;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 4;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Back");
				lcd_put_cur(1, 0);
				lcd_send_string("");
				break;

			case 6:
				if(menu_click){
					current_page = 4;
					menu_click = 0;
					mode_manual(0);
					menu_func(1, 0);
				}else{
					current_page = 6;
					menu_up = 0;
					menu_down = 0;
					lcd_init ();
					lcd_send_string ("Watering");
					lcd_put_cur(1, 0);
					lcd_send_string("Now");
				}
				break;

			case 7:
				if(menu_click){
					current_page = 10;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 8;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 13;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Line 1");
				lcd_put_cur(1, 0);
				lcd_send_string("  Line 2");
				break;

			case 8:
				if(menu_click){
					current_page = 10;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 9;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 7;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Line 2");
				lcd_put_cur(1, 0);
				lcd_send_string("  Time");
				break;

			case 9:
				if(menu_click){
					current_page = 11;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 12;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 8;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Time");
				lcd_put_cur(1, 0);
				lcd_send_string("  Start");
				break;

			case 10:
				if(menu_click){
					current_page = 7;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else{
					current_page = 10;
					lcd_init ();
					lcd_send_string ("> 20");
					lcd_put_cur(1, 0);
					lcd_send_string("  ");
				}
				break;

			case 11:
				if(menu_click){
					current_page = 9;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else{
					current_page = 11;
					lcd_init ();
					lcd_send_string ("> Clock Loop");
					lcd_put_cur(1, 0);
					lcd_send_string("  ");
				}
				break;

			case 12:
				if(menu_click){
					current_page = 1;
					menu_click = 0;
					global_planner_start = 1;
					mode_planner(0, global_planner_start);
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 13;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 9;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Start");
				lcd_put_cur(1, 0);
				lcd_send_string("  Back");
				break;

			case 13:
				if(menu_click){
					current_page = 1;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 7;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 12;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Back");
				lcd_put_cur(1, 0);
				lcd_send_string(" ");
				break;

			case 14:
				if(menu_click){
					current_page = 2;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else{
					current_page = 14;
					lcd_init ();
					itoa(inst_temperature, inst_temperature_s,10);
					itoa(inst_humidity,inst_humidity_s,10);
					itoa(adc_result_percentage[0],adc_percentage_s0,10);
					lcd_send_string("T:");
					lcd_send_string(inst_temperature_s);
					lcd_send_string(" H:");
					lcd_send_string(inst_humidity_s);
					lcd_send_string(" S1:");
					lcd_send_string(adc_percentage_s0);
					lcd_put_cur(1, 0);
					itoa(adc_result_percentage[1],adc_percentage_s1,10);
					itoa(adc_result_percentage[2],adc_percentage_s2,10);
					itoa(adc_result_percentage[3],adc_percentage_s3,10);
					lcd_send_string(" S2:");
					lcd_send_string(adc_percentage_s1);
					lcd_send_string(" S3:");
					lcd_send_string(adc_percentage_s2);
					lcd_send_string(" S4:");
					lcd_send_string(adc_percentage_s3);
				}
				break;

			case 15:
				if(menu_click){
					current_page = 11;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 16;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 17;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Time S.");
				lcd_put_cur(1, 0);
				lcd_send_string("  Bluetooth");
				break;

			case 16:
				if(menu_click){
					current_page = 11;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 17;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 15;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Bluetooth");
				lcd_put_cur(1, 0);
				lcd_send_string("  Back");
				break;
			case 17:
				if(menu_click){
					current_page = 3;
					menu_click = 0;
					menu_func(1, 0);
					break;
				}else if(menu_down){
					current_page = 15;
					menu_down = 0;
					menu_func(1, 0);
					break;
				}else if(menu_up){
					current_page = 16;
					menu_up = 0;
					menu_func(1, 0);
					break;
				}
				lcd_init ();
				lcd_send_string ("> Back");
				lcd_put_cur(1, 0);
				lcd_send_string("  ");
				break;
		}
		menu_lcd_refresh = 0;
		menu_click= 0;
		menu_up = 0;
		menu_down = 0;
	}
}

/*-----------------MENU END--------------------------*/

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
