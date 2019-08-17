#ifndef _CLI_H
#define _CLI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"

typedef struct
{
   const char *cmd;
   const char *description;
   void (*cmdFunc)(uint8_t argc, char **argv);
}sTermEntry_t;

void cli_handleCommand(char *cmd);


extern const sTermEntry_t hEntry;
extern const sTermEntry_t helpEntry;
extern const sTermEntry_t rebootEntry;
extern const sTermEntry_t bootEntry;
//extern const sTermEntry_t sleepEntry;

#ifdef __cplusplus
}
#endif

#endif
