#ifndef __LogManager
#define __LogManager
#include <Arduino.h>
#include "SPIFFS.h"

class LogManager{

	public:
		static uint64_t total_tips;
		static uint64_t last_tip;

		static void setup();
		static void addPaymentLog( uint32_t amount );

		static char *uint64_to_string(uint64_t input);
		static uint64_t atoll(const char* ptr);

};





#endif

