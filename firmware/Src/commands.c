#include <stdio.h>

#include "Utils/cli.h"
#include "Utils/commands.h"

void rtc_debug(uint8_t argc, char **argv);
const sTermEntry_t rtcEntry =
{ "date", "RTC date", rtc_debug };

void nrf(uint8_t argc, char **argv);
const sTermEntry_t nrfEntry =
{ "n", "NRF status", nrf };

void adc(uint8_t argc, char **argv);
const sTermEntry_t adcEntry =
{ "a", "ADC sample", adc };

void water(uint8_t argc, char **argv);
const sTermEntry_t waterEntry =
{ "w", "Toggle water", water };

const sTermEntry_t *cli_entries[] =
{
      &hEntry,
      &helpEntry,
      &rebootEntry,
      &bootEntry,
	  &rtcEntry,
	  &nrfEntry,
	  &adcEntry,
	  &waterEntry,
      0
};
