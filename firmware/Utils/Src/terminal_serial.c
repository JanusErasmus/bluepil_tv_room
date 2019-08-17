 /******************************************************************************
  * @file    vcom.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    27-February-2017
  * @brief   manages virtual com port
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"

#include "Utils/terminal_serial.h"
#include "Utils/terminal.h"
#include "Utils/utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFSIZE 256
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(const char * file, int line)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    printf(RED("Error @%s line: %d"), file, line);
    while (1)
    {
    }
}

/* Uart Handle */
UART_HandleTypeDef UartHandle;
volatile uint8_t vcomOK = 0;

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

void terminal_serial_write(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[128];
	vsnprintf(buffer, 128, format, args);
	printf(buffer);
	va_end(args);
}

void terminal_serial_Init(void)
{
    // if the com is already initialized return
    if(!vcomOK)
    {
        /*## Configure the UART peripheral ######################################*/
        /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
        /* UART1 configured as follow:
         - Word Length = 8 Bits
         - Stop Bit = One Stop bit
         - Parity = ODD parity
         - BaudRate = 921600 baud
         - Hardware flow control disabled (RTS and CTS signals)
         */
        UartHandle.Instance        = VCOM_USART;

        UartHandle.Init.BaudRate   = 115200;
        UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
        UartHandle.Init.StopBits   = UART_STOPBITS_1;
        UartHandle.Init.Parity     = UART_PARITY_NONE;
        UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        UartHandle.Init.Mode       = UART_MODE_TX_RX;

        if(HAL_UART_Init(&UartHandle) != HAL_OK)
        {
            /* Initialization Error */
            _Error_Handler(__FILE__, __LINE__);
        }

        VCOM_USART->CR1 |= USART_CR1_RXNEIE;

        HAL_NVIC_SetPriority(VCOM_USART_IRQn, 0x1, 0);
        HAL_NVIC_EnableIRQ(VCOM_USART_IRQn);

        /* This allows printf to work without \n */
        setbuf(stdout, 0);

        vcomOK = 1;
    }
}

//void terminal_serial_Start(eVcomStates_t state)
//{
//   mVcomState = state;
//
//   switch (state)
//   {
//      case VCOM_NORMAL:
//      {
//         mActive = true;
//         mTimingOut = true;
//         printf("VCOM started\n");
//         printf(GREEN("Press enter to enable Terminal..\n"));
//      }
//      break;
//      case VCOM_DEBUG:
//      {
//         mActive = true;
//         mTimingOut = false;
//         printf("VCOM started\n");
//         printf(GREEN("Type s to sleep terminal.\n"));
//      }
//      break;
//      case VCOM_OFF:
//      {
//         mActive = false;
//         printf(RED("VCOM disabled.\n"));
//      }
//      break;
//   }
//}

void terminal_serial_DisableRx(void)
{
    /* Clear the interrupt enable */
    VCOM_USART->CR1 &= ~USART_CR1_RXNEIE;
}

void terminal_serial_DeInit(void)
{
   vcomOK = 0;

#if 1
  HAL_UART_DeInit(&UartHandle);
#endif
}


/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/

  /* Enable USART1 clock */
  VCOM_USART_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  terminal_serial_IoInit( );
}


void terminal_serial_handleByte(uint8_t byte)
{
	terminal_handleByte(byte);
}

int terminal_serial_ready()
{
	return vcomOK;
}

int terminal_serial_transmit(uint8_t *buf, int len)
{
	if(HAL_UART_Transmit(&UartHandle, buf, len, 300)  != HAL_OK)
		return -1;

	return len;
}

void terminal_serial_IoInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct={0};
    /* Enable GPIO TX/RX clock */
  VCOM_USART_TX_GPIO_CLK_ENABLE();
  VCOM_USART_RX_GPIO_CLK_ENABLE();
    /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = VCOM_USART_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(VCOM_USART_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = VCOM_USART_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(VCOM_USART_RX_GPIO_PORT, &GPIO_InitStruct);
}

void terminal_serial_IoDeInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure={0};

   VCOM_USART_TX_GPIO_CLK_ENABLE();
   VCOM_USART_RX_GPIO_CLK_ENABLE();

   GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
   GPIO_InitStructure.Pull = GPIO_NOPULL;

   GPIO_InitStructure.Pin =  VCOM_USART_TX_PIN ;
   HAL_GPIO_Init(  VCOM_USART_TX_GPIO_PORT, &GPIO_InitStructure );

   GPIO_InitStructure.Pin =  VCOM_USART_RX_PIN ;
   HAL_GPIO_Init(  VCOM_USART_RX_GPIO_PORT, &GPIO_InitStructure );

   VCOM_USART_CLK_DISABLE();
}


/**
  * @brief UART MSP DeInit
  * @param huart: uart handle
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  terminal_serial_IoDeInit( );
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
