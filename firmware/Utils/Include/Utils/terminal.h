#ifndef KSES_UTILITIES_TERMINAL_H_
#define KSES_UTILITIES_TERMINAL_H_
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	int (*isReady)(void);
	int (*transmit_cb)(uint8_t *buf, int len);
}sTerminalInterface_t;

void terminal_init(sTerminalInterface_t **interfaces);

void terminal_run(void);
void terminal_handleByte(uint8_t byte);
#ifdef __cplusplus
}
#endif
#endif /* KSES_UTILITIES_TERMINAL_H_ */
