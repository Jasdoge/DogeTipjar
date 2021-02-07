#ifndef __ConnectionManager
#define __ConnectionManager
#include <Arduino.h>
#include <WiFi.h>

class ConnectionManager{

	public:
		static void setup();
		static bool autoConnect( String ssid, String password, void(*connectionTick)(uint8_t tick) );

};



#endif

