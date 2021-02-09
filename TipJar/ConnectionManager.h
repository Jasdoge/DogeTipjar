#ifndef __ConnectionManager
#define __ConnectionManager
#include <Arduino.h>
#include <WiFi.h>

class ConnectionManager{

	public:
		static bool connected;

		static void setup( void(*onDisconnectCallback)() );
		static void loop();

		static bool autoConnect( String ssid, String password, void(*connectionTick)(uint8_t tick) );
		
	private:
		static void (*onDisconnect)();

};



#endif

