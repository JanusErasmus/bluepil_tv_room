#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/terminal.h"
#include "Utils/cli.h"
#include "stm32f1xx_hal.h"

//static sTerminal_t *terminals[] = {0,0}; //support up to 2 terminals
#define CMD_BUFFER_SIZE     128
#define VCOM_DELAY   		2*1000 //mS

static char mCmdBuffer[CMD_BUFFER_SIZE];
static uint8_t mCmdHead = 0;
static uint8_t mCmdTail = 0;

static char mCommand[CMD_BUFFER_SIZE];
static uint8_t mCmdIndex = 0;

static sTerminalInterface_t **mInterfaces = 0;

void parseByte(char byte)
{
	//echo typed character
	printf("%c", byte);
	if (byte == '\r')
	{
		printf("%c", '\n');
	}

	if (byte == '\r') // || (byte == '\n'))
	{
		mCommand[mCmdIndex++] = byte;
		mCommand[mCmdIndex++] = 0;

		cli_handleCommand(mCommand);
		mCmdIndex = 0;
	}
	else
		mCommand[mCmdIndex++] = byte;

	if (mCmdIndex >= CMD_BUFFER_SIZE)
		mCmdIndex = 0;
}

void terminal_run()
{
	while (mCmdHead != mCmdTail)
	{
		parseByte(mCmdBuffer[mCmdTail]);
		mCmdTail = (mCmdTail + 1) % CMD_BUFFER_SIZE;
	}
}

void terminal_handleByte(uint8_t byte)
{
	mCmdBuffer[mCmdHead] = byte;
	mCmdHead = (mCmdHead + 1) % CMD_BUFFER_SIZE;
}

void terminal_init(sTerminalInterface_t **interfaces)
{
	mInterfaces = interfaces;

	printf("Terminal started\n");
}

int _write(int file, char *data, int len)
{
	if(!mInterfaces)
		return -1;

	int k = 0;
	while(mInterfaces[k])
	{
		if(mInterfaces[k]->isReady())
		{
			mInterfaces[k]->transmit_cb((uint8_t*)data, len);

			if(data[len -1] == '\n')
			{
				uint8_t CR = '\r';

				mInterfaces[k]->transmit_cb(&CR, 1);
			}
		}

		k++;
	}

   return len;
}

