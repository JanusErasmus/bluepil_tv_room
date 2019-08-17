#include "Utils/cli.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Utils/utils.h"
#include "Utils/commands.h"

extern void WWDG_Refresh();

/*!
 * Private functions
 */
static uint8_t parseCommand(char *command, uint8_t argc, char **argv)
{
    uint8_t count = 0;
    argc -= 2;
    argv[count] = command;
    char *ptr = strchr(argv[count], ' ');

    while (ptr && (count++ <= argc))
    {
        ptr[0] = 0;
        ptr++;

        argv[count] = ptr;

        ptr = strchr(argv[count], ' ');
    }

    count++;

    return count;
}

void strip_whiteSpaces(char *cmd)
{
    int len = strlen(cmd);
    for(uint32_t idx = 0; idx < len; idx++)
    {
        if((cmd[idx] == '\n') || (cmd[idx] == '\r'))
        {
            cmd[idx] = 0;
            return;
        }
    }
}

void cli_handleCommand(char *cmd)
{
    strip_whiteSpaces(cmd);

    uint8_t processed = 0;
    char* argv[6];
    uint8_t argc = 0;
    uint8_t cmdIndex = 0;
    argc = parseCommand(cmd, 6, argv);
    if (argc)
    {
        const sTermEntry_t *entry = cli_entries[cmdIndex++];
        while (entry)
        {
            if (!strcmp(entry->cmd, cmd))
            {
                entry->cmdFunc(argc, argv);
                processed = 1;
                break;
            }
            entry = cli_entries[cmdIndex++];
        }
    }

    if (!processed && (strlen(cmd) > 0))
    {
        printf(RED("Unknown command '%s', try help\n"), cmd);
    }

    printf("pill$ ");
}

/*!
 * Debug entries
 */

void help(uint8_t argc, char **argv)
{
    uint8_t cmdIndex = 0;
    const sTermEntry_t *entry = cli_entries[cmdIndex++];

    printf(YELLOW("Available commands:\n"));
    while (entry)
    {
    	printf("%s\t- %s\n", entry->cmd, entry->description);
        entry = cli_entries[cmdIndex++];
    }
}
const sTermEntry_t hEntry =
{ "h", "This help", help };

const sTermEntry_t helpEntry =
{ "help", "This help", help };

void reboot(uint8_t argc, char **argv)
{
	printf("Rebooting...\n");
    NVIC_SystemReset();
}

const sTermEntry_t rebootEntry =
{ "reset", "Reboot the device", reboot };


#include "usb_device.h"


extern TIM_HandleTypeDef htim2;

void jumpBoot(uint8_t argc, char **argv)
{
    printf("Boot mode...\n");

    MX_USB_DEVICE_DeInit();
    HAL_TIM_Base_MspDeInit(&htim2);
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
//
//    GPIO_InitTypeDef GPIO_InitStruct;
//    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12  | GPIO_PIN_11, GPIO_PIN_RESET);

    HAL_Delay(100);


    printf("Bye\n");
    __HAL_RCC_USART1_FORCE_RESET();
    HAL_Delay(5);
    __HAL_RCC_USART1_RELEASE_RESET();
    HAL_Delay(5);

    HAL_RCC_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    typedef void (*funcPtr)(void);
    uint32_t system_memory = 0x1FFFF000;
    uint32_t jumpAddr = *(volatile uint32_t *)(system_memory + 0x04); /* reset ptr in vector table */

    funcPtr usrMain = (funcPtr) jumpAddr;

    SCB->VTOR = (volatile uint32_t)system_memory;

    /* Initialize user application's Stack Pointer */
    //__set_MSP(*(__IO uint32_t*) 0x1FFFF000);
    asm volatile("msr msp, %0"::"g"(*(volatile uint32_t *)system_memory));

    usrMain();
}

const sTermEntry_t bootEntry =
{ "boot", "Jump to bootloader", jumpBoot };

//void sleep(uint8_t argc, char **argv)
//{
//	vcom_sleep();
//}
//
//const sTermEntry_t sleepEntry =
//{ "s", "Sleep the terminal", sleep };

/*!
 * Public functions
 */

char *currentCommand = 0;

HAL_StatusTypeDef cli_setCommand(char *cmd)
{
	if(currentCommand)
		return HAL_ERROR;

	currentCommand = cmd;
	return HAL_OK;
}



