#ifndef __ConnectionManager
#define __ConnectionManager
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

class ConnectionManager{

	public:
		static bool connected;
		

		static void setup( void(*onDisconnectCallback)() );
		static void loop();

		static time_t getTime();

		static bool autoConnect( String ssid, String password, void(*connectionTick)(uint8_t tick) );
		
	private:
		static void (*onDisconnect)();
		static struct tm timeinfo;

};



#endif

