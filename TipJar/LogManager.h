#ifndef __LogManager
#define __LogManager
#include <Arduino.h>
#include "SPIFFS.h"

class LogManager{

	public:
		static uint64_t total_tips;
		static uint64_t last_tip;
		static uint8_t volume;
		static String RECEIVING_ADDRESS;
		static uint64_t address_changed;
		

		static void setup();
		static void addPaymentLog( uint32_t amount );
		static void setVolume( uint8_t volume );
		static void setAddress( String address, uint64_t time );

		static char *uint64_to_string(uint64_t input);
		static uint64_t atoll(const char* ptr);

};





#endif

