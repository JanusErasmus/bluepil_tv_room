#include "main.h"

#include <time.h>

#include "Utils/cli.h"
#include "Utils/crc.h"
#include "Utils/terminal_serial.h"
#include "stm32f1xx_hal.h"

#include "Utils/terminal.h"
#include "Utils/utils.h"
#include "usb_device.h"
#include "interface_nrf24.h"

#define STREET_NODE_ADDRESS     0x00
#define UPS_NODE_ADDRESS        0x01
#define UPS12V_NODE_ADDRESS     0x02
#define FERMENTER_NODE_ADDRESS  0x03
#define HOUSE_NODE_ADDRESS      0x04
#define GARAGE_NODE_ADDRESS     0x05
#define WATER_NODE_ADDRESS      0x06

#define NODE_ADDRESS WATER_NODE_ADDRESS

uint8_t netAddress[] = {0x22, 0xBB, 0x55};
uint8_t serverAddress[] = {0x11, 0xBB, 0x55};
#define payload_length 16

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
SPI_HandleTypeDef hspi1;
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim2;
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
extern "C" {
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_ADC1_Init(void);
}

/* Private function prototypes -----------------------------------------------*/
enum nodeFrameType_e
{
	DATA = 0,
	COMMAND = 1,
	ACKNOWLEDGE = 2
};

//Node send 32 bytes of data, with the last byte being the 8-bit CRC
typedef struct {
	uint8_t nodeAddress;	//1
	uint8_t frameType;		//1
	uint32_t timestamp;		//4  6
	uint8_t inputs;			//1  7
	uint8_t outputs;		//1  8
	uint16_t voltages[4];	//8  16
	uint16_t temperature;	//2  18
	uint8_t reserved[13]; 	//13 31
	uint8_t crc;			//1  32
}__attribute__((packed, aligned(4))) nodeData_s;


bool reportToServer = false;

void setWater(bool state)
{
	//pull solinoid in for 1s
	if(state)
	{
		HAL_GPIO_WritePin(WATER_HOLD_OUT_Port, WATER_HOLD_OUT_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(WATER_OUT_Port, WATER_OUT_Pin, GPIO_PIN_SET);
		HAL_Delay(1000);
		HAL_GPIO_WritePin(WATER_OUT_Port, WATER_OUT_Pin, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(WATER_HOLD_OUT_Port, WATER_HOLD_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(WATER_OUT_Port, WATER_OUT_Pin, GPIO_PIN_RESET);
	}

}

uint32_t getADCstep()
{
	uint32_t adc = 0;
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = ADC_CHANNEL_VREFINT;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_ADC_Start(&hadc1);
	if(HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	{
		adc = HAL_ADC_GetValue(&hadc1);
		//printf("REF	: %d\n", adc);
	}
	HAL_ADC_Stop(&hadc1);

	//this amount of steps measure 1.2V
	uint32_t step = 1200000000 / adc;
	//printf("step %d\n", (int)step);
	return step;
}

uint32_t sampleTemperature()
{
	uint32_t adc = 0;

	ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

	HAL_ADC_Start(&hadc1);
	if(HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	{
		adc = HAL_ADC_GetValue(&hadc1);
		//printf("ADC: %d\n", adc);
	}
	HAL_ADC_Stop(&hadc1);


	return adc;
}

uint32_t getTemperature()
{
	uint32_t sum = 0;

	HAL_ADCEx_Calibration_Start(&hadc1);

	for (int k = 0; k < 8; ++k) {
		sum += sampleTemperature();
	}

	uint32_t adc = sum >> 3;
	//printf("adc: %d\n" , (int)adc);
	int voltage = adc * getADCstep();//845666; //x10^9
	//printf(" *	%d\n", voltage);
	voltage = 1.43e9 - voltage;
	//printf(" -	%d\n", voltage);
	voltage /= 4.3e3;
	//printf(" /	%d\n", voltage);

	uint32_t temp = 25000 + voltage;
	//printf("temp: %d\n", (int)temp);
	return temp;
}

void report(uint8_t *address)
{
//	printf("NOT reporting\n");

	nodeData_s pay;
	memset(&pay, 0, 32);
	pay.nodeAddress = NODE_ADDRESS;
	pay.timestamp = HAL_GetTick();
	pay.temperature = getTemperature();
	pay.voltages[0] = 2;

	if(HAL_GPIO_ReadPin(WATER_HOLD_OUT_Port, WATER_HOLD_OUT_Pin) == GPIO_PIN_SET)
		pay.voltages[0] = 1;

	pay.crc = CRC_8::crc((uint8_t*)&pay, 31);

	//report status in voltages[0-1]
	printf("TX result %d\n", InterfaceNRF24::get()->transmit(address, (uint8_t*)&pay, 32));
}

void reportNow()
{
	report(serverAddress);
}

bool NRFreceivedCB(int pipe, uint8_t *data, int len)
{
	if(CRC_8::crc(data, 32))
	{
		printf(RED("CRC error\n"));
		return false;
	}

	bool reportNow = false;
	nodeData_s down;
	memcpy(&down, data, len);

	printf("RX 0x%02X\n", down.nodeAddress);

	//Check of this is not my data
	if(down.nodeAddress != NODE_ADDRESS)
	{
		if(down.nodeAddress == 0xFF)
		{
			reportNow = true;
		}
		else
			return false;
	}

	if(down.frameType == ACKNOWLEDGE)
	{
		printf("Main: " GREEN("ACK\n"));
		return false;
	}

	printf("RCV Type# %d\n", (int)down.frameType);
	//printf(" PAYLOAD: %d\n", len);
	//diag_dump_buf(data, len);

	int hour = (down.timestamp >> 8) & 0xFF;
	int min = (down.timestamp) & 0xFF;
	printf("Set time %d:%d\n", hour, min);

	RTC_TimeTypeDef sTime;
	sTime.Hours = hour;
	sTime.Minutes = min;
	sTime.Seconds = 0;
	HAL_StatusTypeDef result = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	if(result != HAL_OK)
		printf("Could not set Time!!! %d\n", result);


	int month = (down.timestamp >> 24) & 0xFF;
	int day = (down.timestamp >> 16) & 0xFF;
	printf("Set date %d:%d\n", month, day);

	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	sDate.Month = month;
	sDate.Date = day;
	result = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	if(result != HAL_OK)
		printf("Could not set Date!!! %d\n", result);

	//Broadcast pipe
	if(reportNow)
	{
		reportToServer = true;
	}

	//command to node
	if(down.frameType == COMMAND)
	{
		printf("Set Outputs %d\n", down.outputs);

	}

	return false;
}

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

 // DrivewayMotors motors(&lights, &streetGate, &houseGate);

  HAL_Delay(1000);

  MX_USB_DEVICE_Init();
  sTerminalInterface_t usb = {
		  MX_USB_DEVICE_ready,
		  MX_USB_DEVICE_transmit
    };

  terminal_serial_Init();
  sTerminalInterface_t serial = {
		  terminal_serial_ready,
		  terminal_serial_transmit
  };

  sTerminalInterface_t *interfaces[] = {
		  &serial,
		  &usb,
		  0
  };

  terminal_init((sTerminalInterface_t **)&interfaces);

  MX_SPI1_Init();
  MX_ADC1_Init();

  if(HAL_GPIO_ReadPin(NRF_ADDR0_GPIO_Port, NRF_ADDR0_Pin) == GPIO_PIN_RESET)
	  netAddress[0] |= 0x01;

  if(HAL_GPIO_ReadPin(NRF_ADDR1_GPIO_Port, NRF_ADDR1_Pin) == GPIO_PIN_RESET)
	  netAddress[0] |= 0x02;

  InterfaceNRF24::init(&hspi1, netAddress, 3);
  InterfaceNRF24::get()->setRXcb(NRFreceivedCB);

  printf("Bluepill @ %dHz\n", (int)HAL_RCC_GetSysClockFreq());
  MX_RTC_Init();


//  report(netAddress);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /* Infinite loop */
  while (1)
  {
	  InterfaceNRF24::get()->run();
	  terminal_run();

	  if(reportToServer)
	  {
		  //before transmitting wait 200 ms intervals of node address
		  HAL_Delay(200 + (NODE_ADDRESS * 200));
		  reportNow();
		  reportToServer = false;
	  }

      HAL_Delay(100);
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  }

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* RTC init function */
static void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

//  RCC->APB1ENR |= (RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN);


    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.State = HAL_RTC_STATE_RESET;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
//  RTC_TamperTypeDef tamper;
//  tamper.Tamper = RTC_TAMPER_1;
//  tamper.Trigger = RTC_TAMPERTRIGGER_HIGHLEVEL;
//  HAL_RTCEx_SetTamper(&hrtc, &tamper);
  HAL_RTCEx_DeactivateTamper(&hrtc, RTC_TAMPER_1);

    /**Initialize RTC and set the Time and Date 
    */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x32F2)
  {
	  printf(GREEN("RTC: "));

	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	  printf("RTC date");
	  printf(" - %04d-%02d-%02d ", 2000 + sDate.Year, sDate.Month, sDate.Date);
	  printf("%02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
  }
  else
  {
	  printf(RED("RTC: Not set\n"));
	  sTime.Hours = 0;
	  sTime.Minutes = 0;
	  sTime.Seconds = 0;

	  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	  {
		  _Error_Handler(__FILE__, __LINE__);
	  }

	  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	  sDate.Year = 0;
	  sDate.Month = RTC_MONTH_JANUARY;
	  sDate.Date = 0;

	  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	  {
		  _Error_Handler(__FILE__, __LINE__);
	  }

	  HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2);
  }
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin : SPI1_CS_Pin */
	GPIO_InitStruct.Pin = SPI1_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF_CE_Pin */
	GPIO_InitStruct.Pin = NRF_CE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(NRF_CE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF_IRQ_Pin */
	GPIO_InitStruct.Pin = NRF_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF_ADDR0_Pin */
	GPIO_InitStruct.Pin = NRF_ADDR0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(NRF_ADDR0_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF_ADDR1_Pin */
	GPIO_InitStruct.Pin = NRF_ADDR1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(NRF_ADDR1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = WATER_OUT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(WATER_OUT_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(WATER_OUT_Port, WATER_OUT_Pin, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = WATER_HOLD_OUT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(WATER_HOLD_OUT_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(WATER_HOLD_OUT_Port, WATER_HOLD_OUT_Pin, GPIO_PIN_RESET);


}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE	;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

/* TIM2 init function */

const char *getDayName(int week_day)
{
	switch(week_day)
	{
	case RTC_WEEKDAY_MONDAY:
		return "Monday";
	case RTC_WEEKDAY_TUESDAY:
		return "Tuesday";
	case RTC_WEEKDAY_WEDNESDAY:
		return "Wednesday";
	case RTC_WEEKDAY_THURSDAY:
		return "Thursday";
	case RTC_WEEKDAY_FRIDAY:
		return "Friday";
	case RTC_WEEKDAY_SATURDAY:
		return "Saturday";
	case RTC_WEEKDAY_SUNDAY:
		return "Sunday";
	}

	return 0;
}

#ifdef __cplusplus
 extern "C" {
#endif

void nrf(uint8_t argc, char **argv)
{
	if(InterfaceNRF24::get())
	{
		uint8_t address[5];
		memcpy(address, serverAddress, 5);

		if(argc > 1)
		{
			address[0] = strtoul(argv[1], 0, 16);
		}

		report(address);

	}
}

void adc(uint8_t argc, char **argv)
{
	uint32_t temp = getTemperature();
	printf("temp: %dmC\n", (int)temp);
}


void water(uint8_t argc, char **argv)
{
	static bool waterState = false;
	printf("Toggle water\n");

	if(waterState)
	{
		waterState = false;
		setWater(false);
	}
	else
	{
		waterState = true;
		setWater(true);
	}
}


void rtc_debug(uint8_t argc, char **argv)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	if(argc > 5)
	{
		printf("Setting date %d\n", atoi(argv[5]));

		sDate.WeekDay = RTC_WEEKDAY_MONDAY;
		sDate.Year = atoi(argv[1]) - 2000;
		sDate.Month = atoi(argv[2]);
		sDate.Date = atoi(argv[3]);
		sTime.Hours = atoi(argv[4]);
		sTime.Minutes = atoi(argv[5]);
		sTime.Seconds = 0;

		RCC->APB1ENR |= (RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN);
		//PWR->CR |= PWR_CR_DBP;
		HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	}


	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	printf("RTC date: %s %d\n", getDayName(sDate.WeekDay), (int)HAL_RTC_SecondsSinceEpoch(sDate, sTime));
	printf(" - %04d-%02d-%02d ", 2000 +sDate.Year, sDate.Month, sDate.Date);
	printf("%02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
}

#ifdef __cplusplus
 }
#endif


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif


/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

